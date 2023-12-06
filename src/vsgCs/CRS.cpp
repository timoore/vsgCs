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

#include "CRS.h"

#include "runtimeSupport.h"

#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>

namespace vsgCs
{
    vsg::dvec3 EPSG4979::getECEF(const vsg::dvec3& coord)
    {
        using namespace CesiumGeospatial;
        auto glmvec = Ellipsoid::WGS84.cartographicToCartesian(Cartographic(coord.x, coord.y, coord.z));
        return glm2vsg(glmvec);
    }

    vsg::dmat4 EPSG4979::getENU(const vsg::dvec3& coord)
    {
        using namespace CesiumGeospatial;
        LocalHorizontalCoordinateSystem localSys(Cartographic(coord.x, coord.y, coord.z));
        vsg::dmat4 localMat = glm2vsg(localSys.getLocalToEcefTransformation());
        return localMat;
    }
}
