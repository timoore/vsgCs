#include "vsgResourcePreparer.h"
#include "CompilableImage.h"

#include <Cesium3DTilesSelection/GltfUtilities.h>
#include <Cesium3DTilesSelection/Tile.h>
#include <CesiumAsync/AsyncSystem.h>


using namespace vsgCs;
using namespace CesiumGltf;


LoadModelResult*
vsgResourcePreparer::readAndCompile(Cesium3DTilesSelection::TileLoadResult &&tileLoadResult,
                                    const glm::dmat4& transform,
                                    const CreateModelOptions& options)
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (!ref_viewer)
        return nullptr;
    auto resultNode = _builder->loadTile(std::move(tileLoadResult), transform, options);
    LoadModelResult* result = new LoadModelResult;
    result->modelResult = resultNode;
    result->compileResult = ref_viewer->compileManager->compile(resultNode);
    return result;
}

RenderResources* merge(vsgResourcePreparer* preparer, LoadModelResult& result)
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = preparer->viewer;
    if (ref_viewer)
    {
        updateViewer(*ref_viewer, result.compileResult);
        return new RenderResources{result.modelResult};
    }
    return nullptr;
}

CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
vsgResourcePreparer::prepareInLoadThread(const CesiumAsync::AsyncSystem& asyncSystem,
                                         Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
                                         const glm::dmat4& transform,
                                         const std::any&)
{
    CesiumGltf::Model* pModel = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);
    if (!pModel)
    {
        return asyncSystem.createResolvedFuture(
            Cesium3DTilesSelection::TileLoadResultAndRenderResources{
                std::move(tileLoadResult),
                nullptr});
    }

    CreateModelOptions options;
    LoadModelResult* result = readAndCompile(std::move(tileLoadResult), transform, options);
    return asyncSystem.createResolvedFuture(
        Cesium3DTilesSelection::TileLoadResultAndRenderResources{
            std::move(tileLoadResult),
            result});
}

void*
vsgResourcePreparer::prepareInMainThread(Cesium3DTilesSelection::Tile& tile,
                                         void* pLoadThreadResult)
{
    const Cesium3DTilesSelection::TileContent& content = tile.getContent();
    if (content.isRenderContent())
    {
        LoadModelResult* loadModelResult = reinterpret_cast<LoadModelResult*>(pLoadThreadResult);
        return merge(this, *loadModelResult);
    }
    return nullptr;
}

void vsgResourcePreparer::free(Cesium3DTilesSelection::Tile&,
                               void* pLoadThreadResult,
                               void* pMainThreadResult) noexcept
{
    if (pLoadThreadResult)
    {
        LoadModelResult* loadModelResult = reinterpret_cast<LoadModelResult*>(pLoadThreadResult);
        delete loadModelResult;
    }
    else if (pMainThreadResult)
    {
        RenderResources* renderResources = reinterpret_cast<RenderResources*>(pMainThreadResult);
        delete renderResources;
    }
}

void*
vsgResourcePreparer::prepareRasterInLoadThread(CesiumGltf::ImageCesium& image,
                                               const std::any&)
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (!ref_viewer)
        return nullptr;
    auto result = _builder->loadTexture(image,
                                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                        VK_FILTER_LINEAR,
                                        VK_FILTER_LINEAR,
                                        true,
                                        true);
    auto compilable = CompilableImage::create(result);
    return new LoadRasterResult{compilable->imageInfo, ref_viewer->compileManager->compile(compilable)};
}

void*
vsgResourcePreparer::prepareRasterInMainThread(Cesium3DTilesSelection::RasterOverlayTile&,
                                               void* rawLoadResult)
{
    LoadRasterResult* loadRasterResult = static_cast<LoadRasterResult*>(rawLoadResult);
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (ref_viewer)
    {
        vsg::updateViewer(*ref_viewer, loadRasterResult->compileResult);
    }

    return new RasterResources{loadRasterResult->rasterResult};
}

void
vsgResourcePreparer::freeRaster(const Cesium3DTilesSelection::RasterOverlayTile&,
                                void* loadThreadResult,
                                void* mainThreadResult) noexcept
{

    LoadRasterResult* loadRasterResult = static_cast<LoadRasterResult*>(loadThreadResult);
    delete loadRasterResult;
    RasterResources* rasterResources = static_cast<RasterResources*>(mainThreadResult);
    delete rasterResources;
}

void
vsgResourcePreparer::attachRasterInMainThread(const Cesium3DTilesSelection::Tile& tile,
                                  int32_t overlayTextureCoordinateID,
                                  const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                  void* pMainThreadRendererResources,
                                  const glm::dvec2& translation,
                                  const glm::dvec2& scale)
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (!ref_viewer)
        return;
    const Cesium3DTilesSelection::TileContent& content = tile.getContent();
    const Cesium3DTilesSelection::TileRenderContent* renderContent = content.getRenderContent();
    if (renderContent)
    {
        RenderResources* resources = static_cast<RenderResources*>(renderContent->getRenderResources());

        auto object = _builder->attachRaster(tile, resources->model,
                                             overlayTextureCoordinateID, rasterTile,
                                             pMainThreadRendererResources, translation, scale);
        if (object.valid())
        {
            auto compileResult = ref_viewer->compileManager->compile(object);
            vsg::updateViewer(*ref_viewer, compileResult);
        }
    }
}

void
vsgResourcePreparer::detachRasterInMainThread(const Cesium3DTilesSelection::Tile& tile,
                                              int32_t overlayTextureCoordinateID,
                                              const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                              void*) noexcept
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (!ref_viewer)
        return;

    const Cesium3DTilesSelection::TileContent& content = tile.getContent();
    const Cesium3DTilesSelection::TileRenderContent* renderContent = content.getRenderContent();
    if (renderContent)
    {
        RenderResources* resources = static_cast<RenderResources*>(renderContent->getRenderResources());
        auto object = _builder->detachRaster(tile, resources->model, overlayTextureCoordinateID, rasterTile);
        if (object.valid())
        {
            auto compileResult = ref_viewer->compileManager->compile(object);
            vsg::updateViewer(*ref_viewer, compileResult);
        }
    }
}
