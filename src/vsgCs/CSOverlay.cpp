#include "CSOverlay.h"

#include "Cesium3DTilesSelection/IonRasterOverlay.h"
#include <vsg/all.h>

using namespace vsgCs;

void CSOverlay::addToTileset(vsg::ref_ptr<TilesetNode> tilesetNode)
{
    Cesium3DTilesSelection::Tileset* tileset = tilesetNode->getTileset();
    Cesium3DTilesSelection::RasterOverlayOptions options{};
    options.maximumScreenSpaceError = this->MaximumScreenSpaceError;
    options.maximumSimultaneousTileLoads = this->MaximumSimultaneousTileLoads;
    options.maximumTextureSize = this->MaximumTextureSize;
    options.subTileCacheBytes = this->SubTileCacheBytes;
    options.showCreditsOnScreen = this->ShowCreditsOnScreen;
    options.loadErrorCallback =
        [this](const Cesium3DTilesSelection::RasterOverlayLoadFailureDetails&
                 details) {
        uint8_t typeValue = uint8_t(details.type);
        assert(
            uint8_t(details.type) <=
            uint8_t(
                Cesium3DTilesSelection::RasterOverlayLoadType::TileProvider));
        // assert(this->_pTileset == details.pTileset);
        vsg::warn(details.message);
        };
      std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> overlay =
          createOverlay(options);
      if (overlay)
      {
          _rasterOverlay.swap(overlay);
          tileset->getOverlays().add(_rasterOverlay.get());
      }
}

std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> CSIonRasterOverlay::createOverlay(
            const Cesium3DTilesSelection::RasterOverlayOptions& options)
{
      if (this->IonAssetID <= 0)
      {
          // Don't create an overlay for an invalid asset ID.
          return nullptr;
      }
      return std::make_unique<Cesium3DTilesSelection::IonRasterOverlay>(
          MaterialLayerKey,
          IonAssetID,
          IonAccessToken,
          options);
}
