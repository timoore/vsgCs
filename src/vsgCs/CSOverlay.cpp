#include "CSOverlay.h"
#include "OpThreadTaskProcessor.h"

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
    _rasterOverlay = createOverlay(options);
      if (_rasterOverlay)
      {
          tileset->getOverlays().add(_rasterOverlay);
      }
}

void CSOverlay::removeFromTileset(vsg::ref_ptr<TilesetNode> tilesetNode)
{
    ++_overlaysBeingDestroyed;
    Cesium3DTilesSelection::Tileset* tileset = tilesetNode->getTileset();
    _rasterOverlay->getAsyncDestructionCompleteEvent(getAsyncSystem())
      .thenInMainThread([this]() { --this->_overlaysBeingDestroyed; });
    tileset->getOverlays().remove(_rasterOverlay);
    _rasterOverlay = 0;
}

Cesium3DTilesSelection::RasterOverlay* CSIonRasterOverlay::createOverlay(
            const Cesium3DTilesSelection::RasterOverlayOptions& options)
{
      if (this->IonAssetID <= 0)
      {
          // Don't create an overlay for an invalid asset ID.
          return nullptr;
      }
      return new Cesium3DTilesSelection::IonRasterOverlay(
          MaterialLayerKey,
          IonAssetID,
          IonAccessToken,
          options);
}
