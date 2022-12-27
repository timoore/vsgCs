#pragma once

#include <vsg/all.h>
#include <Cesium3DTilesSelection/IPrepareRendererResources.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>

#include "LoadGltfResult.h"
#include "CesiumGltfBuilder.h"

namespace vsgCs
{
    class VSGCS_EXPORT vsgResourcePreparer : public Cesium3DTilesSelection::IPrepareRendererResources
    {
    public:
        vsgResourcePreparer(vsg::ref_ptr<vsg::Options> vsgOptions, vsg::ref_ptr<vsg::Viewer> viewer = {})
    : viewer(viewer), _builder(CesiumGltfBuilder::create(vsgOptions))
    {
    }
    
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

    void* prepareRasterInMainThread(Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                    void* pLoadThreadResult) override;

    void freeRaster(const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                    void* pLoadThreadResult,
                    void* pMainThreadResult) noexcept override;

    void attachRasterInMainThread(const Cesium3DTilesSelection::Tile& tile,
                                  int32_t overlayTextureCoordinateID,
                                  const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                  void* pMainThreadRendererResources,
                                  const glm::dvec2& translation,
                                  const glm::dvec2& scale) override;

    void detachRasterInMainThread(const Cesium3DTilesSelection::Tile& tile,
                                  int32_t overlayTextureCoordinateID,
                                  const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                  void* pMainThreadRendererResources) noexcept override;
    vsg::observer_ptr<vsg::Viewer> viewer;
    protected:
    LoadModelResult* readAndCompile(Cesium3DTilesSelection::TileLoadResult &&tileLoadResult,
                                    const glm::dmat4& transform,
                                    const CreateModelOptions& options);
    vsg::ref_ptr<CesiumGltfBuilder> _builder;
    };
}
