#include "vsgResourcePreparer.h"

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
vsgResourcePreparer::prepareRasterInLoadThread(CesiumGltf::ImageCesium&,
                                               const std::any&)
{
    return nullptr;
}

void*
vsgResourcePreparer::prepareRasterInMainThread(Cesium3DTilesSelection::RasterOverlayTile&,
                                               void*)
{
    return nullptr;
}

void
vsgResourcePreparer::freeRaster(const Cesium3DTilesSelection::RasterOverlayTile&,
                                void*,
                                void*) noexcept
{
}

void
vsgResourcePreparer::attachRasterInMainThread(const Cesium3DTilesSelection::Tile&,
                                              int32_t,
                                              const Cesium3DTilesSelection::RasterOverlayTile&,
                                              void*,
                                              const glm::dvec2&,
                                              const glm::dvec2&)
{
}

void
vsgResourcePreparer::detachRasterInMainThread(const Cesium3DTilesSelection::Tile&,
                                              int32_t,
                                              const Cesium3DTilesSelection::RasterOverlayTile&,
                                              void*) noexcept
{
}

