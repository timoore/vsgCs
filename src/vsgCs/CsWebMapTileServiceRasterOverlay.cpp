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
    if (this->BaseUrl.empty())
    {
        // Don't create an overlay with an empty base URL.
        return nullptr;
    }

    CesiumRasterOverlays::WebMapTileServiceRasterOverlayOptions wmtsOptions;
    if (!Style.empty())
    {
        wmtsOptions.style = (this->Style);
    }
    if (!Layer.empty())
    {
        wmtsOptions.layer = (this->Layer);
    }
    if (!Format.empty())
    {
        wmtsOptions.format = (this->Format);
    }
    if (!TileMatrixSetID.empty())
    {
        wmtsOptions.tileMatrixSetID = (this->TileMatrixSetID);
    }

    if (bSpecifyZoomLevels && MaximumLevel > MinimumLevel)
    {
        wmtsOptions.minimumLevel = MinimumLevel;
        wmtsOptions.maximumLevel = MaximumLevel;
    }

    wmtsOptions.tileWidth = this->TileWidth;
    wmtsOptions.tileHeight = this->TileHeight;

    const CesiumGeospatial::Ellipsoid& ellipsoid = options.ellipsoid;

    if (this->Projection ==
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
                RectangleWest,
                RectangleSouth,
                RectangleEast,
                RectangleNorth);
        CesiumGeometry::Rectangle coverageRectangle =
            CesiumGeospatial::projectRectangleSimple(
                *wmtsOptions.projection,
                globeRectangle);
        wmtsOptions.coverageRectangle = coverageRectangle;
        wmtsOptions.tilingScheme = CesiumGeometry::QuadtreeTilingScheme(
            coverageRectangle,
            RootTilesX,
            RootTilesY);
    }

    if (bSpecifyTileMatrixSetLabels)
    {
        if (!TileMatrixSetLabels.empty())
        {
            std::vector<std::string> labels;
            for (const auto& label : this->TileMatrixSetLabels)
            {
                labels.emplace_back((label));
            }
            wmtsOptions.tileMatrixLabels = labels;
        }
    }
    else
    {
        if (!TileMatrixSetLabelPrefix.empty())
        {
            std::vector<std::string> labels;
            for (size_t level = 0; level <= 25; ++level)
            {
                std::string label(TileMatrixSetLabelPrefix);
                label.append(std::to_string(level));
                labels.emplace_back((label));
            }
            wmtsOptions.tileMatrixLabels = labels;
        }
    }

    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;

    for (const auto& [Key, Value] : this->RequestHeaders)
    {
        headers.push_back(CesiumAsync::IAssetAccessor::THeader{
            (Key),
            (Value) });
    }

    return new CesiumRasterOverlays::WebMapTileServiceRasterOverlay(
        (this->MaterialLayerKey),
        (this->BaseUrl),
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
        overlay->BaseUrl = CesiumUtility::JsonHelpers::getStringOrDefault(json, "baseUrl", "");
        overlay->Layer = CesiumUtility::JsonHelpers::getStringOrDefault(json, "layer", "");
        overlay->Style = CesiumUtility::JsonHelpers::getStringOrDefault(json, "style", "");
        overlay->Format = CesiumUtility::JsonHelpers::getStringOrDefault(json, "format", "");
        overlay->TileMatrixSetID = CesiumUtility::JsonHelpers::getStringOrDefault(json, "tileMatrixSetID", "");
        std::string projection = CesiumUtility::JsonHelpers::getStringOrDefault(json, "projection", "");
        overlay->Projection = projection == "geographic" ? ECesiumWebMapTileServiceRasterOverlayProjection::Geographic : ECesiumWebMapTileServiceRasterOverlayProjection::WebMercator;

        return overlay;
    }

    // XXX See jsonUtils for registration.
}
