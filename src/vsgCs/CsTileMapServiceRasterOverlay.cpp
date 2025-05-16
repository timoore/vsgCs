﻿/* <editor-fold desc="MIT License">

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
    if (url.empty())
    {
        // Don't create an overlay with an empty URL.
        return nullptr;
    }

    CesiumRasterOverlays::TileMapServiceRasterOverlayOptions tmsOptions;
    if (maximumLevel > minimumLevel && bSpecifyZoomLevels)
    {
        tmsOptions.minimumLevel = minimumLevel;
        tmsOptions.maximumLevel = maximumLevel;
    }

    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;

    for (const auto& [Key, Value] : requestHeaders)
    {
        headers.push_back(CesiumAsync::IAssetAccessor::THeader{
            Key,
            Value });
    }

    return new CesiumRasterOverlays::TileMapServiceRasterOverlay(
        MaterialLayerKey,
        url,
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
        overlay->url = CesiumUtility::JsonHelpers::getStringOrDefault(json, "url", "");
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
