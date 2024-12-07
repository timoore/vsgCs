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

#include <vsg/nodes/Group.h>
// See comments elsewhere about this Windows brokeness.
#ifndef NOGDI
#define NOGDI
#endif
#include <Cesium3DTilesSelection/Tileset.h>
#include <gsl/span>

#include "vsgCs/Export.h"
#include "runtimeSupport.h"
#include "jsonUtils.h"

namespace vsgCs
{
    class VSGCS_EXPORT WorldNode : public vsg::Inherit<vsg::Group, WorldNode>
    {
        public:
        WorldNode();
        // Init (create TilesetNode objects) from Json data
        void init(const rapidjson::Value& worldJson, JSONObjectFactory* factory = nullptr);
        /**
         * @brief Load tilesets, set up recurring VSG tasks, and perform any other necessary
         * initialization. This calls updateViews().
         *
         * Call this after the initial command graphs have been assigned to the viewer, but before
         * the main loop starts.
         */
        bool initialize(const vsg::ref_ptr<vsg::Viewer>& viewer);
        void shutdown();
        // hack for supporting zoom after load
        const Cesium3DTilesSelection::Tile* getRootTile(size_t tileset = 0);
        protected:
        vsg::Group::Children& worldNodes()
        {
            auto stateGroup = ref_ptr_cast<vsg::StateGroup>(children[0]);
            return stateGroup->children;
        }
    };
}
