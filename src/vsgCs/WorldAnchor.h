/* <editor-fold desc="MIT License">

Copyright(c) 2024 Timothy Moore

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
#include "Export.h"

#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <string>

namespace vsgCs
{
    // A node that serves as a parent to the world node, in order to bring the world into an
    // application's coordinate system.
    // crs - Coordinate Reference System
    // in_worldOrigin : The geographic or projected coordinates of the world origin
    // in_localOrigin :  The Cartesion coordinates that the world origin will have
    //
    // Specifying these two origins allows us to use round Earth terrain in an area of interest with
    // minimized error

    class VSGCS_EXPORT WorldAnchor : public vsg::Inherit<vsg::MatrixTransform, WorldAnchor>
    {
    public:
        WorldAnchor(const std::string& crs,
                    const vsg::dvec3& in_worldOrigin,
                    const vsg::dvec3& in_localOrigin = vsg::dvec3(0.0, 0.0, 0.0));
        std::shared_ptr<CRS> crs;
        const vsg::dvec3 worldOrigin;
        const vsg::dvec3 localOrigin;
        // The Earth North Up transform for the anchor point i.e., the inverse of the matrix transform.
        vsg::dmat4 enu;
    };
}

