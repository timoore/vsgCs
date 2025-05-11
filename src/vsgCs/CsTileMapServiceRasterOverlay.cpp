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

#include "CsTileMapServiceRasterOverlay.h"

#include "jsonUtils.h"
#include <CesiumRasterOverlays/TileMapServiceRasterOverlay.h>
#include <CesiumUtility/JsonHelpers.h>

using namespace vsgCs;

CesiumRasterOverlays::RasterOverlay* CsTileMapServiceRasterOverlay::createOverlay(
    const CesiumRasterOverlays::RasterOverlayOptions& options)
{
    if (this->Url.empty())
    {
        // Don't create an overlay with an empty URL.
        return nullptr;
    }

    CesiumRasterOverlays::TileMapServiceRasterOverlayOptions tmsOptions;
    if (MaximumLevel > MinimumLevel && bSpecifyZoomLevels)
    {
        tmsOptions.minimumLevel = MinimumLevel;
        tmsOptions.maximumLevel = MaximumLevel;
    }

    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;

    for (const auto& [Key, Value] : this->RequestHeaders)
    {
        headers.push_back(CesiumAsync::IAssetAccessor::THeader{
            (Key),
            (Value) });
    }

    return new CesiumRasterOverlays::TileMapServiceRasterOverlay(
        (this->MaterialLayerKey),
        (this->Url),
        headers,
        tmsOptions,
        options);
}

namespace vsgCs
{
    vsg::ref_ptr<vsg::Object> buildCsTileMapServiceRasterOverlay(const rapidjson::Value& json,
        JSONObjectFactory* factory,
        const vsg::ref_ptr<vsg::Object>& object)
    {
        auto overlay = create_or<CsTileMapServiceRasterOverlay>(object);
        factory->build(json, "CsOverlay", overlay);
        overlay->MaterialLayerKey = CesiumUtility::JsonHelpers::getStringOrDefault(json, "materialKey",
            "Overlay0");
        overlay->Url = CesiumUtility::JsonHelpers::getStringOrDefault(json, "url", "");

        return overlay;
    }

    // XXX See jsonUtils for registration.
}
