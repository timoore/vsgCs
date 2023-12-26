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

#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>

// Some include file in Cesium (actually, it's spdlog.h) unleashes the hell of windows.h. We need to
// turn off GDI defines to avoid a redefinition of the GLSL constant OPAQUE.
#ifndef NOGDI
#define NOGDI
#endif

#include "CesiumRasterOverlays/RasterOverlay.h"
#include "Export.h"
#include "TilesetNode.h"

namespace vsgCs
{
    class VSGCS_EXPORT CsOverlay : public vsg::Inherit<vsg::Object, CsOverlay>
    {
    public:
        std::string MaterialLayerKey = "Overlay0";
        float MaximumScreenSpaceError = 2.0;
        int32_t MaximumTextureSize = 2048;
        int32_t MaximumSimultaneousTileLoads = 20;
        int64_t SubTileCacheBytes = 16 * 1024 * 1024;
        // The number of this layer in a tileset's layers. Layers are rendered from highest number
        // to lowest.
        uint32_t layerNumber = 0;
        /**
         * @brief overall alpha value for the overlay.
         */
        float alpha = 1.0;
        bool ShowCreditsOnScreen;
        // I followed cesium-unreal's lead by making this an action to perform on an overlay; why not
        // have TilesetNode do this?
        void addToTileset(const vsg::ref_ptr<TilesetNode>& tilesetNode);
        void removeFromTileset(const vsg::ref_ptr<TilesetNode>& tilesetNode);
        virtual CesiumRasterOverlays::RasterOverlay* createOverlay(
            const CesiumRasterOverlays::RasterOverlayOptions& options = {}) = 0;
        CesiumRasterOverlays::RasterOverlay* getOverlay()
        {
            return _rasterOverlay;
        }
        const  CesiumRasterOverlays::RasterOverlay* getOverlay() const
        {
            return _rasterOverlay;
        }
    protected:
        CesiumRasterOverlays::RasterOverlay* _rasterOverlay;
        int32_t _overlaysBeingDestroyed = 0;
    };

    class VSGCS_EXPORT CsIonRasterOverlay : public vsg::Inherit<CsOverlay, CsIonRasterOverlay>
    {
        public:
        CsIonRasterOverlay(int64_t in_IonAssetID = -1,
                           std::string in_IonAccessToken = std::string())
            : IonAssetID(in_IonAssetID), IonAccessToken(std::move(in_IonAccessToken))
        {}
        int64_t IonAssetID;
        std::string IonAccessToken;
        CesiumRasterOverlays::RasterOverlay* createOverlay(
            const CesiumRasterOverlays::RasterOverlayOptions& options) override;
    };
}
