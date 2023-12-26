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

#include "CsOverlay.h"
#include "jsonUtils.h"
#include "OpThreadTaskProcessor.h"
#include "RuntimeEnvironment.h"
#include "runtimeSupport.h"

#include <CesiumRasterOverlays/IonRasterOverlay.h>
#include <CesiumUtility/JsonHelpers.h>
#include <vsg/all.h>

using namespace vsgCs;

void CsOverlay::addToTileset(const vsg::ref_ptr<TilesetNode>& tilesetNode)
{
    CesiumRasterOverlays::RasterOverlayOptions options{};
    options.maximumScreenSpaceError = this->MaximumScreenSpaceError;
    options.maximumSimultaneousTileLoads = this->MaximumSimultaneousTileLoads;
    options.maximumTextureSize = this->MaximumTextureSize;
    options.subTileCacheBytes = this->SubTileCacheBytes;
    options.showCreditsOnScreen = this->ShowCreditsOnScreen;
    options.loadErrorCallback =
        [](const CesiumRasterOverlays::RasterOverlayLoadFailureDetails&
                 details) {
            assert(
                int(details.type) <=
                int(
                    CesiumRasterOverlays::RasterOverlayLoadType::TileProvider));
            vsg::warn(details.message);
        };
    options.rendererOptions = OverlayRendererOptions{layerNumber, alpha};
    _rasterOverlay = createOverlay(options);
      if (_rasterOverlay)
      {
          tilesetNode->addOverlay(vsg::ref_ptr<CsOverlay>(this));
      }
}

void CsOverlay::removeFromTileset(const vsg::ref_ptr<TilesetNode>& tilesetNode)
{
    ++_overlaysBeingDestroyed;
    _rasterOverlay->getAsyncDestructionCompleteEvent(getAsyncSystem())
      .thenInMainThread([this]() { --this->_overlaysBeingDestroyed; });
    tilesetNode->removeOverlay(vsg::ref_ptr<CsOverlay>(this));
    _rasterOverlay = nullptr;
}

CesiumRasterOverlays::RasterOverlay* CsIonRasterOverlay::createOverlay(
            const CesiumRasterOverlays::RasterOverlayOptions& options)
{
      if (this->IonAssetID <= 0)
      {
          // Don't create an overlay for an invalid asset ID.
          return nullptr;
      }
      return new CesiumRasterOverlays::IonRasterOverlay(
          MaterialLayerKey,
          IonAssetID,
          IonAccessToken,
          options);
}

namespace
{
    vsg::ref_ptr<vsg::Object> buildCsOverlay(const rapidjson::Value& json,
                                             JSONObjectFactory*,
                                             const vsg::ref_ptr<vsg::Object>& object)
    {
        if (!object)
        {
            throw std::runtime_error("Can't directly create a CsOverlay object.");
        }
        auto overlay = ref_ptr_cast<CsOverlay>(object);
        overlay->alpha = static_cast<float>(CesiumUtility::JsonHelpers::getDoubleOrDefault(json, "alpha", 1.0));
        return object;
    }

    JSONObjectFactory::Registrar rCsOverlay("CsOverlay", buildCsOverlay);

    vsg::ref_ptr<vsg::Object> buildCSIonRasterOverlay(const rapidjson::Value& json,
                                                      JSONObjectFactory* factory,
                                                      const vsg::ref_ptr<vsg::Object>& object)
    {
        auto env = RuntimeEnvironment::get();
        auto overlay = create_or<CsIonRasterOverlay>(object);
        if (!overlay)
        {
            throw std::runtime_error("no valid CSIonOverlay obejct");
        }
        factory->build(json, "CsOverlay", overlay);
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
