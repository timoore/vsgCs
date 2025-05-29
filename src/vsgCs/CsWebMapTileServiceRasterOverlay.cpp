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

#include "CsWebMapTileServiceRasterOverlay.h"

#include "jsonUtils.h"
#include <CesiumRasterOverlays/WebMapTileServiceRasterOverlay.h>
#include <CesiumUtility/JsonHelpers.h>

using namespace vsgCs;

CesiumRasterOverlays::RasterOverlay* CsWebMapTileServiceRasterOverlay::createOverlay(
    const CesiumRasterOverlays::RasterOverlayOptions& options)
{
    if (baseUrl.empty())
    {
        // Don't create an overlay with an empty base URL.
        return nullptr;
    }

    CesiumRasterOverlays::WebMapTileServiceRasterOverlayOptions wmtsOptions;
    if (!style.empty())
    {
        wmtsOptions.style = style;
    }
    if (!layer.empty())
    {
        wmtsOptions.layer = layer;
    }
    if (!format.empty())
    {
        wmtsOptions.format = format;
    }
    if (!tileMatrixSetID.empty())
    {
        wmtsOptions.tileMatrixSetID = tileMatrixSetID;
    }

    if (bSpecifyZoomLevels && maximumLevel > minimumLevel)
    {
        wmtsOptions.minimumLevel = minimumLevel;
        wmtsOptions.maximumLevel = maximumLevel;
    }

    wmtsOptions.tileWidth = tileWidth;
    wmtsOptions.tileHeight = tileHeight;

    const CesiumGeospatial::Ellipsoid& ellipsoid = options.ellipsoid;

    if (projection ==
        ECesiumWebMapTileServiceRasterOverlayProjection::Geographic)
    {
        wmtsOptions.projection = CesiumGeospatial::GeographicProjection(ellipsoid);
    }
    else
    {
        wmtsOptions.projection = CesiumGeospatial::WebMercatorProjection(ellipsoid);
    }

    if (bSpecifyTilingScheme)
    {
        CesiumGeospatial::GlobeRectangle globeRectangle =
            CesiumGeospatial::GlobeRectangle::fromDegrees(
                rectangleWest,
                rectangleSouth,
                rectangleEast,
                rectangleNorth);
        CesiumGeometry::Rectangle coverageRectangle =
            CesiumGeospatial::projectRectangleSimple(
                *wmtsOptions.projection,
                globeRectangle);
        wmtsOptions.coverageRectangle = coverageRectangle;
        wmtsOptions.tilingScheme = CesiumGeometry::QuadtreeTilingScheme(
            coverageRectangle,
            rootTilesX,
            rootTilesY);
    }

    if (bSpecifyTileMatrixSetLabels)
    {
        if (!tileMatrixSetLabels.empty())
        {
            std::vector<std::string> labels;
            for (const auto& label : tileMatrixSetLabels)
            {
                labels.emplace_back(label);
            }
            wmtsOptions.tileMatrixLabels = labels;
        }
    }
    else
    {
        if (!tileMatrixSetLabelPrefix.empty())
        {
            std::vector<std::string> labels;
            for (size_t level = 0; level <= 25; ++level)
            {
                std::string label(tileMatrixSetLabelPrefix);
                label.append(std::to_string(level));
                labels.emplace_back(label);
            }
            wmtsOptions.tileMatrixLabels = labels;
        }
    }

    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;

    for (const auto& [Key, Value] : requestHeaders)
    {
        headers.push_back(CesiumAsync::IAssetAccessor::THeader{
            Key,
            Value });
    }

    return new CesiumRasterOverlays::WebMapTileServiceRasterOverlay(
        MaterialLayerKey,
        baseUrl,
        headers,
        wmtsOptions,
        options);
}

namespace vsgCs
{
    vsg::ref_ptr<vsg::Object> buildCsWebMapTileServiceRasterOverlay(const rapidjson::Value& json,
        JSONObjectFactory* factory,
        const vsg::ref_ptr<vsg::Object>& object)
    {
        auto overlay = create_or<CsWebMapTileServiceRasterOverlay>(object);
        factory->build(json, "CsOverlay", overlay);
        overlay->MaterialLayerKey = CesiumUtility::JsonHelpers::getStringOrDefault(json, "materialKey",
            "Overlay0");
        overlay->baseUrl = CesiumUtility::JsonHelpers::getStringOrDefault(json, "baseUrl", "");
        overlay->layer = CesiumUtility::JsonHelpers::getStringOrDefault(json, "layer", "");
        overlay->style = CesiumUtility::JsonHelpers::getStringOrDefault(json, "style", "");
        overlay->format = CesiumUtility::JsonHelpers::getStringOrDefault(json, "format", overlay->format);
        overlay->tileMatrixSetID = CesiumUtility::JsonHelpers::getStringOrDefault(json, "tileMatrixSetID", "");
        overlay->tileMatrixSetLabelPrefix = CesiumUtility::JsonHelpers::getStringOrDefault(json, "tileMatrixSetLabelPrefix", "");
        std::string projection = CesiumUtility::JsonHelpers::getStringOrDefault(json, "projection", "");
        overlay->projection = projection == "geographic" ? ECesiumWebMapTileServiceRasterOverlayProjection::Geographic : ECesiumWebMapTileServiceRasterOverlayProjection::WebMercator;
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
