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

#include "CRS.h"
#include "vsgCs/Export.h"
#include "RuntimeEnvironment.h"
#include "WorldAnchor.h"

#include <vsg/core/observer_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>

#include <memory>
#include <string>

namespace vsgCs
{
    class VSGCS_EXPORT GeoNode : public vsg::Inherit<vsg::MatrixTransform, GeoNode>
    {
    public:
        GeoNode(const std::string& crs = "epsg:4979", vsg::ref_ptr<WorldAnchor> worldAnchor = {});
        GeoNode(const std::shared_ptr<CRS>& crs, vsg::ref_ptr<WorldAnchor> worldAnchor = {});
        void setOrigin(const vsg::dvec3& origin);
        vsg::dvec3 getOrigin() const;
        void setCRS(const std::string& crs);
        std::shared_ptr<CRS> getCRS()
        {
            return _crs;
        }
        vsg::ref_ptr<WorldAnchor> getWorldAnchor()
        {
            return _worldAnchor;
        }
        void setWorldAnchor(const vsg::ref_ptr<WorldAnchor>& worldAnchor)
        {
            _worldAnchor = worldAnchor;
        }
    protected:
        std::shared_ptr<CRS> _crs;
        vsg::dvec3 _origin;
        vsg::observer_ptr<WorldAnchor> _worldAnchor;
    };

    vsg::ref_ptr<vsg::StateGroup> VSGCS_EXPORT createModelRoot(const vsg::ref_ptr<RuntimeEnvironment>& env);
}
