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

#include "GeospatialServices.h"
#include "runtimeSupport.h"

#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>

#include <vsg/maths/transform.h>

using namespace vsgCs;

CsGeospatialServices::CsGeospatialServices(const vsg::ref_ptr<vsg::Node>& worldNode)
    : _worldNode(worldNode)
{
    auto radii = glm2vsg(CesiumGeospatial::Ellipsoid::WGS84.getRadii());
    auto re = radii.x;
    auto rp = radii.z;
    _ellipsoidToUnitSphere[0][0] = 1.0 / re;
    _ellipsoidToUnitSphere[1][1] = 1.0 / re;
    _ellipsoidToUnitSphere[2][2] = 1.0 / rp;
    _unitSphereToEllipsoid[0][0] = re;
    _unitSphereToEllipsoid[1][1] = re;
    _unitSphereToEllipsoid[2][2] = rp;
}

vsg::dmat4 CsGeospatialServices::localToWorldMatrix(const vsg::dvec3& worldPos)
{
    CesiumGeospatial::LocalHorizontalCoordinateSystem lhcs(glm::dvec3(worldPos.x, worldPos.y, worldPos.z));
    return glm2vsg(lhcs.getLocalToEcefTransformation());
}

bool CsGeospatialServices::isGeocentric()
{
    return true;
}

double CsGeospatialServices::semiMajorAxis()
{
    return CesiumGeospatial::Ellipsoid::WGS84.getMaximumRadius();
}

vsg::dbox CsGeospatialServices::bounds()
{
    auto radii = glm2vsg(CesiumGeospatial::Ellipsoid::WGS84.getRadii());
    return vsg::dbox(radii * -1.0, radii);
}

// Thank you rocky

std::optional<vsg::dvec3> CsGeospatialServices::intersectGeocentricLine(const vsg::dvec3& p0_world,
                                                                        const vsg::dvec3& p1_world)
{
    using namespace vsg;
    double dist2 = 0.0;
    dvec3 v;
    dvec3 p0 = p0_world * _ellipsoidToUnitSphere;
    dvec3 p1 = p1_world * _ellipsoidToUnitSphere;

    constexpr double R = 1.0; // for unit sphere.

    // http://paulbourke.net/geometry/circlesphere/index.html#linesphere

    dvec3 d = p1 - p0;

    double A = dot(d, d); //d * d;
    double B = 2.0 * dot(d, p0); // (d * p0);
    double C = dot(p0, p0) - R * R; // (p0 * p0) - R * R;

    // now solve the quadratic A + B*t + C*t^2 = 0.
    double D = B * B - 4.0*A*C;
    if (D > 0)
    {
        // two roots (line passes through sphere twice)
        // find the closer of the two.
        double sqrtD = sqrt(D);
        double t0 = (-B + sqrtD) / (2.0*A);
        double t1 = (-B - sqrtD) / (2.0*A);

        //seg; pick closest:
        if (fabs(t0) < fabs(t1))
            v = d * t0;
        else
            v = d * t1;
    }
    else if (D == 0.0)
    {
        // one root (line is tangent to sphere?)
        double t = -B / (2.0*A);
        v = d * t;
    }

    dist2 = dot(v, v); // v.length2();

    if (dist2 > 0.0)
    {
        return (p0 + v) * _unitSphereToEllipsoid;
    }
    else
    {
        // either no intersection, or the distance was not the max.
        return {};
    }
}

vsg::ref_ptr<vsg::Node> CsGeospatialServices::getWorldNode()
{
    return _worldNode;
}
