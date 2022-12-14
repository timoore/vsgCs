#include "vsgResourcePreparer.h"
#include "LoadGltfResult.h"

using namespace vsgCs;
using namespace CesiumGltf;


LoadModelResult* readAndCompile(const glm::dmat4& transform, CreateModelOptions& options)
{
    const Model& model = *options.pModel;
    vsg::ref_ptr<vsg::Group> resultNode = vsg::Group::create();
    glm::dmat4x4 rootTransform = transform;
    
    rootTransform
        = Cesium3DTilesSelection::GltfUtilities::applyRtcCenter(model, rootTransform);
    applyGltfUpAxisTransform(model, rootTransform);
    auto transformNode = vsg::MatrixTransform::create(glm2vsg(rootTransform));
    CesiumGltfBuilder builder(options.pModel);
    resultNode = builder::load();
    transformNode->addChild(resultNode);
    // Compile
}

RenderResources* merge(vsgResourcePreparer* preparer, LoadGltfResult::LoadModelResult& result)
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = preparer->viewer;
    if (ref_viewer)
    {
        updateViewer(*ref_viewer, result.compileResult);
        return new RenderResources{result.modelResult};
    }
}

CesiumAsync::Future<TileLoadResultAndRenderResources>
vsgResourcePreparer::prepareInLoadThread(const CesiumAsync::AsyncSystem& asyncSystem,
                                         TileLoadResult&& tileLoadResult,
                                         const glm::dmat4& transform,
                                         const std::any& rendererOptions)
{
    CesiumGltf::Model* pModel = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);
    if (!pModel)
    {
        return asyncSystem.createResolvedFuture(
            Cesium3DTilesSelection::TileLoadResultAndRenderResources{
                std::move(tileLoadResult),
                nullptr});
    }

    CreateModelOptions options{pModel};
    LoadModelResult* result = readAndCompile(transform, options);
    return asyncSystem.createResolvedFuture(
        Cesium3DTilesSelection::TileLoadResultAndRenderResources{
            std::move(tileLoadResult),
            result});
}

virtual void*
vsgResourcePreparer::prepareInMainThread(Cesium3DTilesSelection::Tile& tile,
                                         void* pLoadThreadResult)
{
    const Cesium3DTilesSelection::TileContent& content = tile.getContent();
    if (content.isRenderContent())
    {
        LoadModelResult* loadModelResult = reinterpret_cast<LoadModelResult*>(pLoadThreadResult);
        const Cesium3DTilesSelection::TileRenderContent& renderContent = *content.getRenderContent();
        return merge(this, *loadModelResult);
    }
    return nullptr;
}

void vsgResourcePreparer::free(Cesium3DTilesSelection::Tile& tile,
                               void* pLoadThreadResult,
                               void* pMainThreadResult)
{
    if (pLoadThreadResult)
    {
        LoadModelResult* loadModelResult = reinterpret_cast<LoadModelResult*>(pLoadThreadResult);
        delete loadModelResult;
    }
    else if (pMainThreadResult)
    {
        RenderResources* renderResources = renderResources<RenderResources>(pMainThreadResult);
        delete pMainThreadResult;
    }
}

void*
vsgResourcePreparer::prepareRasterInLoadThread(CesiumGltf::ImageCesium& image,
                                               const std::any& rendererOptions)
{
    return nullptr;
}

void*
vsgResourcePreparer::prepareRasterInMainThread(Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                               void* pLoadThreadResult)
{
    return nullptr;
}

void
vsgResourcePreparer::freeRaster(const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                void* pLoadThreadResult,
                                void* pMainThreadResult)
{
}

void
vsgResourcePreparer::attachRasterInMainThread(const Cesium3DTilesSelection::Tile& tile,
                                              int32_t overlayTextureCoordinateID,
                                              const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                              void* pMainThreadRendererResources,
                                              const glm::dvec2& translation,
                                              const glm::dvec2& scale)
{
}

void
vsgResourcePreparer::detachRasterInMainThread(const Cesium3DTilesSelection::Tile& tile,
                                              int32_t overlayTextureCoordinateID,
                                              const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                              void* pMainThreadRendererResources)
{
}

