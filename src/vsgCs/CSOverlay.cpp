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
            assert(
                details.type <=
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
