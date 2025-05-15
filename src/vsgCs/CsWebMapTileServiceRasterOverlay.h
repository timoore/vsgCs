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

#pragma once

#include "CsOverlay.h"

namespace vsgCs
{
    /**
     * Specifies the type of projection used for projecting a Web Map Tile Service
     * raster overlay.
     */
    enum class ECesiumWebMapTileServiceRasterOverlayProjection : uint8_t {
        /**
         * The raster overlay is projected using Web Mercator.
         */
        WebMercator,

        /**
         * The raster overlay is projected using a geographic projection.
         */
        Geographic
    };

    class VSGCS_EXPORT CsWebMapTileServiceRasterOverlay : public vsg::Inherit<CsOverlay,
        CsWebMapTileServiceRasterOverlay>
    {
    public:
        CsWebMapTileServiceRasterOverlay(
            std::string in_baseUrl = std::string())
            : baseUrl(std::move(in_baseUrl))
        {
        }

        /**
         * The base URL of the Web Map Tile Service (WMTS).
         * This URL should not include query parameters. For example:
         * https://tile.openstreetmap.org/{TileMatrix}/{TileCol}/{TileRow}.png
         */
        std::string baseUrl;

        /**
         * The layer name for WMTS requests.
         */
        std::string layer;

        /**
         * The style name for WMTS requests.
         */
        std::string style;

        /**
         * The MIME type for images to retrieve from the server.
         */
        std::string format = "image/jpeg";

        /**
         * The tile matrix set identifier for WMTS requests.
         */
        std::string tileMatrixSetID;

        /**
         * The prefix to use for the tile matrix set labels. For instance, setting
         * "EPSG:4326:" as prefix generates label list ["EPSG:4326:0", "EPSG:4326:1",
         * "EPSG:4326:2", ...]
         * Only applicable when "Specify Tile Matrix Set Labels" is false.
         */
        std::string tileMatrixSetLabelPrefix;

        /**
         * Set this to true to specify tile matrix set labels manually. If false, the
         * labels will be constructed from the specified levels and prefix (if one is
         * specified).
         */
        bool bSpecifyTileMatrixSetLabels = false;

        /**
         * The manually specified tile matrix set labels.
         *
         * Only applicable when "Specify Tile Matrix Set Labels" is true.
         */
        std::vector<std::string> tileMatrixSetLabels;

        /**
         * The type of projection used to project the WMTS imagery onto the globe.
         * For instance, EPSG:4326 uses geographic projection and EPSG:3857 uses Web
         * Mercator.
         */
        ECesiumWebMapTileServiceRasterOverlayProjection projection =
            ECesiumWebMapTileServiceRasterOverlayProjection::WebMercator;

        /**
         * Set this to true to specify the quadtree tiling scheme according to the
         * specified root tile numbers and projected bounding rectangle. If false, the
         * tiling scheme will be deduced from the projection.
         */
        bool bSpecifyTilingScheme = false;

        /**
         * The number of tiles corresponding to TileCol, also known as
         * TileMatrixWidth. If specified, this determines the number of tiles at the
         * root of the quadtree tiling scheme in the X direction.
         *
         * Only applicable if "Specify Tiling Scheme" is set to true.
         */
        int32_t rootTilesX = 1;

        /**
         * The number of tiles corresponding to TileRow, also known as
         * TileMatrixHeight. If specified, this determines the number of tiles at the
         * root of the quadtree tiling scheme in the Y direction.
         *
         * Only applicable if "Specify Tiling Scheme" is set to true.
         */
        int32_t rootTilesY = 1;

        /**
         * The west boundary of the bounding rectangle used for the quadtree tiling
         * scheme. Specified in longitude degrees in the range [-180, 180].
         *
         * Only applicable if "Specify Tiling Scheme" is set to true.
         */
        double rectangleWest = -180;

        /**
         * The south boundary of the bounding rectangle used for the quadtree tiling
         * scheme. Specified in latitude degrees in the range [-90, 90].
         *
         * Only applicable if "Specify Tiling Scheme" is set to true.
         */
        double rectangleSouth = -90;

        /**
         * The east boundary of the bounding rectangle used for the quadtree tiling
         * scheme. Specified in longitude degrees in the range [-180, 180].
         *
         * Only applicable if "Specify Tiling Scheme" is set to true.
         */
        double rectangleEast = 180;

        /**
         * The north boundary of the bounding rectangle used for the quadtree tiling
         * scheme. Specified in latitude degrees in the range [-90, 90].
         *
         * Only applicable if "Specify Tiling Scheme" is set to true.
         */
        double rectangleNorth = 90;

        /**
         * Set this to true to directly specify the minimum and maximum zoom levels
         * available from the server. If false, the minimum and maximum zoom levels
         * will be retrieved from the server's tilemapresource.xml file.
         */
        bool bSpecifyZoomLevels = false;

        /**
         * Minimum zoom level.
         *
         * Take care when specifying this that the number of tiles at the minimum
         * level is small, such as four or less. A larger number is likely to result
         * in rendering problems.
         */
        int32_t minimumLevel = 0;

        /**
         * Maximum zoom level.
         */
        int32_t maximumLevel = 25;

        /**
         * The pixel width of the image tiles.
         */
        int32_t tileWidth = 256;

        /**
         * The pixel height of the image tiles.
         */
        int32_t tileHeight = 256;

        /**
         * HTTP headers to be attached to each request made for this raster overlay.
         */
        std::map<std::string, std::string> requestHeaders;

        CesiumRasterOverlays::RasterOverlay* createOverlay(
            const CesiumRasterOverlays::RasterOverlayOptions& options) override;
    };
}
