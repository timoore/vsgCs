#pragma once

#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>

#include "Cesium3DTilesSelection/RasterOverlay.h"
#include "Export.h"
#include "TilesetNode.h"

namespace vsgCs
{
    class VSGCS_EXPORT CSOverlay : public vsg::Inherit<vsg::Object, CSOverlay>
    {
    public:
        std::string MaterialLayerKey = "Overlay0";
        float MaximumScreenSpaceError = 2.0;
        int32_t MaximumTextureSize = 2048;
        int32_t MaximumSimultaneousTileLoads = 20;
        int64_t SubTileCacheBytes = 16 * 1024 * 1024;
        bool ShowCreditsOnScreen;
        void addToTileset(vsg::ref_ptr<TilesetNode> tilesetNode);
        virtual std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> createOverlay(
            const Cesium3DTilesSelection::RasterOverlayOptions& options = {}) = 0;
    protected:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> _rasterOverlay;
    };

    class VSGCS_EXPORT CSIonRasterOverlay : public vsg::Inherit<CSOverlay, CSIonRasterOverlay>
    {
        public:
        CSIonRasterOverlay(int64_t in_IonAssetID, std::string in_IonAccessToken)
            : IonAssetID(in_IonAssetID), IonAccessToken(std::move(in_IonAccessToken))
        {}
        int64_t IonAssetID;
        std::string IonAccessToken;
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> createOverlay(
            const Cesium3DTilesSelection::RasterOverlayOptions& options = {}) override;
    };
}
