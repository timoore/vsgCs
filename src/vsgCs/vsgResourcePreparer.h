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

#pragma once

#include <vsg/all.h>
#include <Cesium3DTilesSelection/IPrepareRendererResources.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>

#include "GraphicsEnvironment.h"
#include "LoadGltfResult.h"
#include "CesiumGltfBuilder.h"

#include <deque>

namespace vsgCs
{
    // Sort of a hack to delay deletion of vsg::Objects for 2 frames, in order to not delete Vulkan
    // objects that are still referenced by active command buffers.
    class DeletionQueue
    {
    public:
        DeletionQueue();
        struct Deletion
        {
            uint64_t frameRemoved;
            vsg::ref_ptr<vsg::Object> object;
        };
        void add(const vsg::ref_ptr<vsg::Viewer>& viewer, const vsg::ref_ptr<vsg::Object>& object);
        // Add A "list" of objects for deletion
        template<typename TSpan>
        void addObjects(vsg::ref_ptr<vsg::Viewer> viewer, TSpan&& span)
        {
            for (auto object : span)
            {
                add(viewer, object);
            }
        }
        // Remove everything from queue.
        void run();
        // Run at this frame
        void run(const vsg::ref_ptr<vsg::Viewer>& viewer);
        uint64_t lastFrameRun;
        std::deque<Deletion> queue;
        // A safe value for this seems to depend on the swapchain minImageCount in obscure
        // ways. Hardwired to 3 for now.
        uint64_t frameDelay;
    };

    struct DeviceFeatures;

    class VSGCS_EXPORT vsgResourcePreparer : public Cesium3DTilesSelection::IPrepareRendererResources
    {
    public:
        vsgResourcePreparer(const vsg::ref_ptr<GraphicsEnvironment>& genv,
                            const vsg::ref_ptr<vsg::Viewer>& viewer = {});
    
        CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
            prepareInLoadThread(const CesiumAsync::AsyncSystem& asyncSystem,
                                Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
                                const glm::dmat4& transform,
                                const std::any& rendererOptions) override;

        void* prepareInMainThread(Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult) override;

        void free(Cesium3DTilesSelection::Tile& tile,
                  void* pLoadThreadResult,
                  void* pMainThreadResult) noexcept override;

        void* prepareRasterInLoadThread(CesiumGltf::ImageCesium& image,
                                        const std::any& rendererOptions) override;

        void* prepareRasterInMainThread(CesiumRasterOverlays::RasterOverlayTile& rasterTile,
                                        void* rawLoadResult) override;

        void freeRaster(const CesiumRasterOverlays::RasterOverlayTile& rasterTile,
                        void* pLoadThreadResult,
                        void* pMainThreadResult) noexcept override;

        void attachRasterInMainThread(const Cesium3DTilesSelection::Tile& tile,
                                      int32_t overlayTextureCoordinateID,
                                      const CesiumRasterOverlays::RasterOverlayTile& rasterTile,
                                      void* pMainThreadRendererResources,
                                      const glm::dvec2& translation,
                                      const glm::dvec2& scale) override;

        void detachRasterInMainThread(const Cesium3DTilesSelection::Tile& tile,
                                      int32_t overlayTextureCoordinateID,
                                      const CesiumRasterOverlays::RasterOverlayTile& rasterTile,
                                      void* pMainThreadRendererResources) noexcept override;
        vsg::observer_ptr<vsg::Viewer> viewer;
        vsg::ref_ptr<GraphicsEnvironment> genv;
    protected:
        LoadModelResult* readAndCompile(Cesium3DTilesSelection::TileLoadResult &&tileLoadResult,
                                        const glm::dmat4& transform,
                                        const CreateModelOptions& options);
        void compileAndDelete(ModifyRastersResult& result);
        vsg::ref_ptr<CesiumGltfBuilder> _builder;
        DeletionQueue _deletionQueue;
    };
}
