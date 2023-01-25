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
#include "jsonUtils.h"
#include "OpThreadTaskProcessor.h"
#include "RuntimeEnvironment.h"
#include "runtimeSupport.h"

#include <Cesium3DTilesSelection/IonRasterOverlay.h>
#include <CesiumUtility/JsonHelpers.h>
#include <vsg/all.h>

using namespace vsgCs;

void CSOverlay::addToTileset(vsg::ref_ptr<TilesetNode> tilesetNode)
{
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
            assert(this->_pTileset == details.pTileset);
            vsg::warn(details.message);
        };
    options.rendererOptions = OverlayRendererOptions{layerNumber};
    _rasterOverlay = createOverlay(options);
      if (_rasterOverlay)
      {
          tilesetNode->addOverlay(vsg::ref_ptr<CSOverlay>(this));
      }
}

void CSOverlay::removeFromTileset(vsg::ref_ptr<TilesetNode> tilesetNode)
{
    ++_overlaysBeingDestroyed;
    _rasterOverlay->getAsyncDestructionCompleteEvent(getAsyncSystem())
      .thenInMainThread([this]() { --this->_overlaysBeingDestroyed; });
    tilesetNode->removeOverlay(vsg::ref_ptr<CSOverlay>(this));
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

namespace
{
    vsg::ref_ptr<vsg::Object> buildCSIonRasterOverlay(const rapidjson::Value& json,
                                                      JSONObjectFactory*,
                                                      vsg::ref_ptr<vsg::Object> object)
    {
        auto env = RuntimeEnvironment::get();
        auto overlay = create_or<CSIonRasterOverlay>(object);
        if (!overlay)
        {
            throw std::runtime_error("no valid CSIonOverlay obejct");
        }
        overlay->MaterialLayerKey = CesiumUtility::JsonHelpers::getStringOrDefault(json, "materialKey",
                                                                                   "Overlay0");
        overlay->IonAssetID = CesiumUtility::JsonHelpers::getInt64OrDefault(json, "assetId", -1);
        overlay->IonAccessToken
            = CesiumUtility::JsonHelpers::getStringOrDefault(json, "accessToken",
                                                             env->ionAccessToken);
        // XXX ignore for the moment
        auto ionEndpointUrl = CesiumUtility::JsonHelpers::getStringOrDefault(json, "server", "");
        return overlay;
    }

    JSONObjectFactory::Registrar r("IonRasterOverlay", buildCSIonRasterOverlay);
}
