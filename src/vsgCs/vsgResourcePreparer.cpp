/* <editor-fold desc="MIT License">

Copyright(c) 2023 Timothy Moore

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

</editor-fold> */

#include "vsgResourcePreparer.h"
#include "CompilableImage.h"

#include <Cesium3DTilesSelection/GltfUtilities.h>
#include <Cesium3DTilesSelection/Tile.h>
#include <CesiumAsync/AsyncSystem.h>

#include <limits>

using namespace vsgCs;
using namespace CesiumGltf;

// This deletion queue is a hack to not delete VSG objects (like buffers) while
// they might still be in use. The problem is that VSG recycles descriptors...

DeletionQueue::DeletionQueue()
    : lastFrameRun(std::numeric_limits<uint64_t>::max())
{
}

void DeletionQueue::add(vsg::ref_ptr<vsg::Viewer> viewer,
                        vsg::ref_ptr<vsg::Object> object)
{
    queue.push_back(Deletion{viewer->getFrameStamp()->frameCount, object});
}

void DeletionQueue::run()
{
    queue.clear();
    lastFrameRun = std::numeric_limits<uint64_t>::max();
}

void DeletionQueue::run(vsg::ref_ptr<vsg::Viewer> viewer)
{
    const auto frameStamp = viewer->getFrameStamp();
    if (lastFrameRun == std::numeric_limits<uint64_t>::max())
    {
        queue.clear();
    }
    else
    {
        if (frameStamp->frameCount <= lastFrameRun)
            return;
        auto itr = queue.begin();
        while (itr != queue.end())
        {
            if (itr->frameRemoved + 2 <= frameStamp->frameCount)
            {
                itr = queue.erase(itr);
            }
            else
            {
                break;
            }
        }
    }
    lastFrameRun = frameStamp->frameCount;
}

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

RenderResources* merge(vsgResourcePreparer* preparer, LoadModelResult& result, vsg::ref_ptr<vsg::Object> attachResult)
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = preparer->viewer;
    if (ref_viewer)
    {
        updateViewer(*ref_viewer, result.compileResult);
        auto attachCompileResult = ref_viewer->compileManager->compile(attachResult);
        vsg::updateViewer(*ref_viewer, attachCompileResult);
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
    options.renderOverlays
        = (tileLoadResult.rasterOverlayDetails
           && !tileLoadResult.rasterOverlayDetails.value().rasterOverlayProjections.empty());
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
    // Cesium doesn't make the Tile object available to the load thread for some reason. Therefore
    // we need to attach the descriptor set with tile-specific data here.
    const Cesium3DTilesSelection::TileContent& content = tile.getContent();
    if (content.isRenderContent())
    {
        LoadModelResult* loadModelResult = reinterpret_cast<LoadModelResult*>(pLoadThreadResult);
        auto attachResult = _builder->attachTileData(tile, loadModelResult->modelResult);
        return merge(this, *loadModelResult, attachResult);
    }
    return nullptr;
}

void vsgResourcePreparer::free(Cesium3DTilesSelection::Tile&,
                               void* pLoadThreadResult,
                               void* pMainThreadResult) noexcept
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    LoadModelResult* loadModelResult = reinterpret_cast<LoadModelResult*>(pLoadThreadResult);
    RenderResources* renderResources = reinterpret_cast<RenderResources*>(pMainThreadResult);

    if (ref_viewer)
    {
        _deletionQueue.run(ref_viewer);
        if (pLoadThreadResult)
        {
            _deletionQueue.add(ref_viewer, loadModelResult->modelResult);
        }
            if (pMainThreadResult)
        if (ref_viewer)
        {
            _deletionQueue.add(ref_viewer, renderResources->model);
        }

    }
    delete loadModelResult;
    delete renderResources;
}

void*
vsgResourcePreparer::prepareRasterInLoadThread(CesiumGltf::ImageCesium& image,
                                               const std::any& rendererOptions)
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
    return new LoadRasterResult{compilable->imageInfo, ref_viewer->compileManager->compile(compilable),
                                std::any_cast<OverlayRendererOptions>(rendererOptions)};
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

    return new RasterResources{loadRasterResult->rasterResult, loadRasterResult->overlayOptions};
}

void
vsgResourcePreparer::freeRaster(const Cesium3DTilesSelection::RasterOverlayTile&,
                                void* loadThreadResult,
                                void* mainThreadResult) noexcept
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    LoadRasterResult* loadRasterResult = static_cast<LoadRasterResult*>(loadThreadResult);
    RasterResources* rasterResources = static_cast<RasterResources*>(mainThreadResult);
    if (ref_viewer)
    {
        _deletionQueue.run(ref_viewer);
        if (loadThreadResult)
        {
            _deletionQueue.add(ref_viewer, loadRasterResult->rasterResult);
        }
            if (mainThreadResult)
        if (ref_viewer)
        {
            _deletionQueue.add(ref_viewer, rasterResources->raster);
        }
    }

    delete loadRasterResult;
    delete rasterResources;
}

void vsgResourcePreparer::compileAndDelete(ModifyRastersResult& result)
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    if (!ref_viewer)
        return;
    for (auto object : result.compileObjects)
    {
        auto compileResult = ref_viewer->compileManager->compile(object);
        vsg::updateViewer(*ref_viewer, compileResult);
    }
    if (!result.deleteObjects.empty())
    {
        _deletionQueue.run(ref_viewer);
        _deletionQueue.addObjects(ref_viewer, result.deleteObjects);
    }
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

        auto results = _builder->attachRaster(tile, resources->model,
                                             overlayTextureCoordinateID, rasterTile,
                                             pMainThreadRendererResources, translation, scale);
        compileAndDelete(results);
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
        auto results = _builder->detachRaster(tile, resources->model, overlayTextureCoordinateID, rasterTile);
        compileAndDelete(results);
    }
}
