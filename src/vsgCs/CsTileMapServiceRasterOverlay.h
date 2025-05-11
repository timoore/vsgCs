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

#pragma once

#include "CsOverlay.h"

namespace vsgCs
{
    class VSGCS_EXPORT CsTileMapServiceRasterOverlay : public vsg::Inherit<CsOverlay,
        CsTileMapServiceRasterOverlay>
    {
    public:
        CsTileMapServiceRasterOverlay(
            std::string in_Url = std::string())
            : Url(in_Url)
        {
        }

        /**
         * The base URL of the Tile Map Service (TMS).
         */
        std::string Url;

        /**
         * True to directly specify minum and maximum zoom levels available from the
         * server, or false to automatically determine the minimum and maximum zoom
         * levels from the server's tilemapresource.xml file.
         */
        bool bSpecifyZoomLevels = false;

        /**
         * Minimum zoom level.
         */
        int32_t MinimumLevel = 0;

        /**
         * Maximum zoom level.
         */
        int32_t MaximumLevel = 10;

        /**
         * HTTP headers to be attached to each request made for this raster overlay.
         */
        std::map<std::string, std::string> RequestHeaders;

        CesiumRasterOverlays::RasterOverlay* createOverlay(
            const CesiumRasterOverlays::RasterOverlayOptions& options) override;
    };
}
