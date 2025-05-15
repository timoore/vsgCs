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
    if (baseUrl.empty())
    {
        // Don't create an overlay with an empty base URL.
        return nullptr;
    }

    CesiumRasterOverlays::WebMapServiceRasterOverlayOptions wmsOptions;
    if (maximumLevel > minimumLevel)
    {
        wmsOptions.minimumLevel = minimumLevel;
        wmsOptions.maximumLevel = maximumLevel;
    }
    wmsOptions.layers = layers;
    wmsOptions.tileWidth = tileWidth;
    wmsOptions.tileHeight = tileHeight;

    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;

    for (const auto& [Key, Value] : requestHeaders)
    {
        headers.push_back(CesiumAsync::IAssetAccessor::THeader{
            Key,
            Value });
    }

    return new CesiumRasterOverlays::WebMapServiceRasterOverlay(
        MaterialLayerKey,
        baseUrl,
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
        overlay->baseUrl = CesiumUtility::JsonHelpers::getStringOrDefault(json, "baseUrl", "");
        overlay->layers = CesiumUtility::JsonHelpers::getStringOrDefault(json, "layers", "");
        overlay->tileWidth = CesiumUtility::JsonHelpers::getInt32OrDefault(json, "tileWidth", overlay->tileWidth);
        overlay->tileHeight = CesiumUtility::JsonHelpers::getInt32OrDefault(json, "tileHeight", overlay->tileHeight);
        overlay->minimumLevel = CesiumUtility::JsonHelpers::getInt32OrDefault(json, "minimumLevel", overlay->minimumLevel);
        overlay->maximumLevel = CesiumUtility::JsonHelpers::getInt32OrDefault(json, "maximumLevel", overlay->maximumLevel);
        const auto requestHeaders = json.FindMember("requestHeaders");
        if (requestHeaders != json.MemberEnd() && requestHeaders->value.IsObject())
        {
            auto obj = requestHeaders->value.GetObject();
            for (auto itr = obj.MemberBegin(); itr != obj.MemberEnd(); ++itr)
            {
                overlay->requestHeaders.insert(std::make_pair(itr->name.GetString(), itr->value.GetString()));
            }
        }

        return overlay;
    }

    // XXX See jsonUtils for registration.
}
