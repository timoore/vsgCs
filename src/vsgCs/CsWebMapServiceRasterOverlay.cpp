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

#include "CsWebMapServiceRasterOverlay.h"

#include "jsonUtils.h"
#include <CesiumRasterOverlays/WebMapServiceRasterOverlay.h>
#include <CesiumUtility/JsonHelpers.h>

using namespace vsgCs;

CesiumRasterOverlays::RasterOverlay* CsWebMapServiceRasterOverlay::createOverlay(
    const CesiumRasterOverlays::RasterOverlayOptions& options)
{
    if (this->BaseUrl.empty())
    {
        // Don't create an overlay with an empty base URL.
        return nullptr;
    }

    CesiumRasterOverlays::WebMapServiceRasterOverlayOptions wmsOptions;
    if (MaximumLevel > MinimumLevel)
    {
        wmsOptions.minimumLevel = MinimumLevel;
        wmsOptions.maximumLevel = MaximumLevel;
    }
    wmsOptions.layers = (Layers);
    wmsOptions.tileWidth = TileWidth;
    wmsOptions.tileHeight = TileHeight;

    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;

    for (const auto& [Key, Value] : this->RequestHeaders)
    {
        headers.push_back(CesiumAsync::IAssetAccessor::THeader{
            (Key),
            (Value) });
    }

    return new CesiumRasterOverlays::WebMapServiceRasterOverlay(
        (this->MaterialLayerKey),
        (this->BaseUrl),
        headers,
        wmsOptions,
        options);
}

namespace vsgCs
{
    vsg::ref_ptr<vsg::Object> buildCsWebMapServiceRasterOverlay(const rapidjson::Value& json,
        JSONObjectFactory* factory,
        const vsg::ref_ptr<vsg::Object>& object)
    {
        auto overlay = create_or<CsWebMapServiceRasterOverlay>(object);
        factory->build(json, "CsOverlay", overlay);
        overlay->MaterialLayerKey = CesiumUtility::JsonHelpers::getStringOrDefault(json, "materialKey",
            "Overlay0");
        overlay->BaseUrl = CesiumUtility::JsonHelpers::getStringOrDefault(json, "baseUrl", "");

        return overlay;
    }

    // XXX See jsonUtils for registration.
}
