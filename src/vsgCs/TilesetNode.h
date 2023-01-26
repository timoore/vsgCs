/* <editor-fold desc="MIT License">

Copyright(c) 2023 Timothy Moore

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#pragma once

#include <vsg/all.h>
#include "Cesium3DTilesSelection/Tileset.h"
#include "Cesium3DTilesSelection/ViewUpdateResult.h"
#include "Export.h"
#include "RuntimeEnvironment.h"
#include "runtimeSupport.h"
#include "vsgResourcePreparer.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>

namespace vsgCs
{
    class CSOverlay;

    struct VSGCS_EXPORT TilesetSource
    {
        std::optional<std::string> url;
        std::optional<int64_t> ionAssetID;
        std::optional<std::string> ionAccessToken;
        std::optional<std::string> ionAssetEndpointUrl;
    };
    
    class VSGCS_EXPORT TilesetNode : public vsg::Inherit<vsg::Node, TilesetNode>
    {
    public:
        static vsg::dmat4 zUp2yUp;
        static vsg::dmat4 yUp2zUp;

        TilesetNode(const DeviceFeatures& deviceFeatures, const TilesetSource& source,
                    const Cesium3DTilesSelection::TilesetOptions& tilesetOptions,
                    vsg::ref_ptr<vsg::Options> options);
        ~TilesetNode() override;

        /**
         * @brief Load the tileset, set up recurring VSG tasks, and perform any other necessary
         * initialization. This calls updateViews().
         *
         * Call this after the initial command graphs have been assigned to the viewer, but before
         * the main loop starts.
         */
        bool initialize(vsg::ref_ptr<vsg::Viewer> viewer);
        /**
         * @brief Call when cameras and views are added to the viewer
         */
        void updateViews(vsg::ref_ptr<vsg::Viewer> viewer);
        // void attachToViewer(vsg::ref_ptr<vsg::Viewer> viewer, vsg::ref_ptr<vsg::Group> attachment);
        void traverse(vsg::Visitor& visitor) override;
        void traverse(vsg::ConstVisitor& visitor) const override;
        void traverse(vsg::RecordTraversal& visitor) const override;
        struct UpdateTileset : public vsg::Inherit<vsg::Operation, UpdateTileset>
        {
            UpdateTileset(vsg::ref_ptr<TilesetNode> in_tilesetNode, vsg::ref_ptr<vsg::Viewer> in_viewer)
                : tilesetNode(in_tilesetNode), viewer(in_viewer)
            {}
            void run() override;
            vsg::observer_ptr<TilesetNode> tilesetNode;
            vsg::observer_ptr<vsg::Viewer> viewer;
        };
        friend struct UpdateTileset;
        void shutdown();
        Cesium3DTilesSelection::Tileset* getTileset()
        {
            return _tileset.get();
        }
        // Add and delete overlays for which the Cesium overlay object has already been created. You
        // probably don't want to call these; use CSOverlay::addTotileset instead.
        void addOverlay(vsg::ref_ptr<CSOverlay> overlay);
        void removeOverlay(vsg::ref_ptr<CSOverlay> overlay);
    protected:
        const Cesium3DTilesSelection::ViewUpdateResult* _viewUpdateResult;
        std::unique_ptr<Cesium3DTilesSelection::Tileset> _tileset;
        std::vector<vsg::ref_ptr<CSOverlay>> _overlays;
    private:
        template<class V> void t_traverse(V& visitor) const;
        int32_t _tilesetsBeingDestroyed;
        
    };
}
