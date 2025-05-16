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
     * Specifies the type of projection used for projecting a URL template
     * raster overlay.
     */
    enum class ECesiumUrlTemplateRasterOverlayProjection : uint8_t {
        /**
         * The raster overlay is projected using Web Mercator.
         */
        WebMercator,

        /**
         * The raster overlay is projected using a geographic projection.
         */
        Geographic
    };

    class VSGCS_EXPORT CsUrlTemplateRasterOverlay : public vsg::Inherit<CsOverlay,
        CsUrlTemplateRasterOverlay>
    {
    public:
        CsUrlTemplateRasterOverlay(
            std::string in_url = std::string())
            : templateUrl(std::move(in_url))
        {
        }

        /**
         * @brief The URL containing template parameters that will be substituted when
         * loading tiles.
         *
         * The following template parameters are supported in `url`:
         * - `{x}` - The tile X coordinate in the tiling scheme, where 0 is the
         * westernmost tile.
         * - `{y}` - The tile Y coordinate in the tiling scheme, where 0 is the
         * nothernmost tile.
         * - `{z}` - The level of the tile in the tiling scheme, where 0 is the root
         * of the quadtree pyramid.
         * - `{reverseX}` - The tile X coordinate in the tiling scheme, where 0 is the
         * easternmost tile.
         * - `{reverseY}` - The tile Y coordinate in the tiling scheme, where 0 is the
         * southernmost tile.
         * - `{reverseZ}` - The tile Z coordinate in the tiling scheme, where 0 is
         * equivalent to `urlTemplateOptions.maximumLevel`.
         * - `{westDegrees}` - The western edge of the tile in geodetic degrees.
         * - `{southDegrees}` - The southern edge of the tile in geodetic degrees.
         * - `{eastDegrees}` - The eastern edge of the tile in geodetic degrees.
         * - `{northDegrees}` - The northern edge of the tile in geodetic degrees.
         * - `{minimumX}` - The minimum X coordinate of the tile's projected
         * coordinates.
         * - `{minimumY}` - The minimum Y coordinate of the tile's projected
         * coordinates.
         * - `{maximumX}` - The maximum X coordinate of the tile's projected
         * coordinates.
         * - `{maximumY}` - The maximum Y coordinate of the tile's projected
         * coordinates.
         * - `{width}` - The width of each tile in pixels.
         * - `{height}` - The height of each tile in pixels.
         */
        std::string templateUrl;

        /**
         * The type of projection used to project the imagery onto the globe.
         * For instance, EPSG:4326 uses geographic projection and EPSG:3857 uses Web
         * Mercator.
         */
        ECesiumUrlTemplateRasterOverlayProjection projection =
            ECesiumUrlTemplateRasterOverlayProjection::WebMercator;

        /**
         * Set this to true to specify the quadtree tiling scheme according to the
         * specified root tile numbers and projected bounding rectangle. If false, the
         * tiling scheme will be deduced from the projection.
         */
        bool bSpecifyTilingScheme = false;

        /**
         * If specified, this determines the number of tiles at the
         * root of the quadtree tiling scheme in the X direction.
         *
         * Only applicable if "Specify Tiling Scheme" is set to true.
         */
        int32_t rootTilesX = 1;

        /**
         * If specified, this determines the number of tiles at the
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
