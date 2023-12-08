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

/* A Coordinate Reference System, or CRS.
   Very much like osgEarth's SRS (Spatial Reference System), but we choose a
   synonym to avoid too much confusion. Also, we mostly care about going from geographic or
   projected coordinates to Earth Centered Earth Fixed (ECEF) and back again.

   A CRS' main function is to convert from its own coordinates to ECEF coordinates and calculate an
   East North Up (ENU) tangent plane at that point.

   It's assumed that the 3D tiles use the WGS84-based Cartesian system (EPSG
   4979). It would be good to look at the tileset metadata and use the
   TILESET_CRS_GEOCENTRIC, if any.
 */

#include "Export.h"

#include <vsg/maths/vec3.h>
#include <vsg/maths/mat4.h>

#include <memory>
#include <string>

namespace vsgCs
{
    class VSGCS_EXPORT CRS
    {
    public:
    CRS(const std::string& name);
    vsg::dvec3 getECEF(const vsg::dvec3& coord);
        // Also known as "localToWorld"; is that a better name for any reason?
    vsg::dmat4 getENU(const vsg::dvec3& coord);
    class ConversionOperation;
    protected:
    std::shared_ptr<ConversionOperation> _converter;
    std::string _name;
    };
}
