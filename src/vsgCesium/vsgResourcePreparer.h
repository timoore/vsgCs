#pragma once

#include <vsg/all.h>
#include <Cesium3DTilesSelection/IPrepareRendererResources.h>

namspace vsgCesium
{
    class VSGCESIUM_EXPORT vsgResourcePreparer : public Cesium3DTilesSelection::IPrepareRendererResources
    {
    public:
    vsgResourcePreparer(vsg::ref_ptr<vsg::Viewer> viewer)
    : viewer(viewer)
    {
    }
    
    CesiumAsync::Future<TileLoadResultAndRenderResources>
    prepareInLoadThread(const CesiumAsync::AsyncSystem& asyncSystem,
                        TileLoadResult&& tileLoadResult,
                        const glm::dmat4& transform,
                        const std::any& rendererOptions) override;

    void* prepareInMainThread(Tile& tile, void* pLoadThreadResult) override;

    void free(Tile& tile,
              void* pLoadThreadResult,
              void* pMainThreadResult) noexcept override;

    void* prepareRasterInLoadThread(CesiumGltf::ImageCesium& image,
                                    const std::any& rendererOptions) override;

    void* prepareRasterInMainThread(RasterOverlayTile& rasterTile,
                                    void* pLoadThreadResult) override;

    void freeRaster(const RasterOverlayTile& rasterTile,
                    void* pLoadThreadResult,
                    void* pMainThreadResult) noexcept override;

    void attachRasterInMainThread(const Tile& tile,
                                  int32_t overlayTextureCoordinateID,
                                  const RasterOverlayTile& rasterTile,
                                  void* pMainThreadRendererResources,
                                  const glm::dvec2& translation,
                                  const glm::dvec2& scale) override;

    void detachRasterInMainThread(const Tile& tile,
                                  int32_t overlayTextureCoordinateID,
                                  const RasterOverlayTile& rasterTile,
                                  void* pMainThreadRendererResources) noexcept override;
    vsg::observer_ptr<vsg::Viewer> viewer;
    
    };
}
