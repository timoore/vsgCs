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
    class VSGCS_EXPORT CsWebMapServiceRasterOverlay : public vsg::Inherit<CsOverlay,
        CsWebMapServiceRasterOverlay>
    {
    public:
        CsWebMapServiceRasterOverlay(
            std::string in_baseUrl = std::string())
            : baseUrl(std::move(in_baseUrl))
        {
        }

        /**
         * The base url of the Web Map Service (WMS).
         * e.g.
         * https://services.ga.gov.au/gis/services/NM_Culture_and_Infrastructure/MapServer/WMSServer
         */
        std::string baseUrl;

        /**
         * Comma-separated layer names to request from the server.
         */
        std::string layers;

        /**
         * Image width
         */
        int32_t tileWidth = 256;

        /**
         * Image height
         */
        int32_t tileHeight = 256;

        /**
         * Minimum zoom level.
         *
         * Take care when specifying this that the number of tiles at the minimum
         * level is small, such as four or less. A larger number is likely to
         * result in rendering problems.
         */
        int32_t minimumLevel = 0;

        /**
         * Maximum zoom level.
         */
        int32_t maximumLevel = 14;

        /**
         * HTTP headers to be attached to each request made for this raster overlay.
         */
        std::map<std::string, std::string> requestHeaders;

        CesiumRasterOverlays::RasterOverlay* createOverlay(
            const CesiumRasterOverlays::RasterOverlayOptions& options) override;
    };
}
