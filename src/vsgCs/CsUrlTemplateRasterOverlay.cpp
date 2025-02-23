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

#include "CsUrlTemplateRasterOverlay.h"

#include "jsonUtils.h"
#include <CesiumRasterOverlays/UrlTemplateRasterOverlay.h>
#include <CesiumUtility/JsonHelpers.h>

using namespace vsgCs;

CesiumRasterOverlays::RasterOverlay* CsUrlTemplateRasterOverlay::createOverlay(
    const CesiumRasterOverlays::RasterOverlayOptions& options)
{
    if (this->TemplateUrl.empty())
    {
        // Don't create an overlay with an empty base URL.
        return nullptr;
    }

    CesiumRasterOverlays::UrlTemplateRasterOverlayOptions urlTemplateOptions;

    urlTemplateOptions.minimumLevel = MinimumLevel;
    urlTemplateOptions.maximumLevel = MaximumLevel;

    urlTemplateOptions.tileWidth = this->TileWidth;
    urlTemplateOptions.tileHeight = this->TileHeight;

    const CesiumGeospatial::Ellipsoid& ellipsoid = options.ellipsoid;

    if (this->Projection ==
        ECesiumUrlTemplateRasterOverlayProjection::Geographic)
    {
        urlTemplateOptions.projection =
            CesiumGeospatial::GeographicProjection(ellipsoid);
    }
    else
    {
        urlTemplateOptions.projection =
            CesiumGeospatial::WebMercatorProjection(ellipsoid);
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
                *urlTemplateOptions.projection,
                globeRectangle);
        urlTemplateOptions.coverageRectangle = coverageRectangle;
        urlTemplateOptions.tilingScheme = CesiumGeometry::QuadtreeTilingScheme(
            coverageRectangle,
            RootTilesX,
            RootTilesY);
    }

    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;

    for (const auto& [Key, Value] : this->RequestHeaders)
    {
        headers.push_back(CesiumAsync::IAssetAccessor::THeader{
            (Key),
            (Value) });
    }

    return new CesiumRasterOverlays::UrlTemplateRasterOverlay(
        (this->MaterialLayerKey),
        (this->TemplateUrl),
        headers,
        urlTemplateOptions,
        options);
}

namespace vsgCs
{
    vsg::ref_ptr<vsg::Object> buildCsUrlTemplateRasterOverlay(const rapidjson::Value& json,
        JSONObjectFactory* factory,
        const vsg::ref_ptr<vsg::Object>& object)
    {
        auto overlay = create_or<CsUrlTemplateRasterOverlay>(object);
        factory->build(json, "CsOverlay", overlay);
        overlay->MaterialLayerKey = CesiumUtility::JsonHelpers::getStringOrDefault(json, "materialKey",
            "Overlay0");
        overlay->TemplateUrl = CesiumUtility::JsonHelpers::getStringOrDefault(json, "url", "");

        return overlay;
    }

    // XXX See jsonUtils for registration.
}
