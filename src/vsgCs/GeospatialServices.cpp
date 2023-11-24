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
    return {radii * -1.0, radii};
}

struct QuadraticResult
{
    int numRoots;
    double roots[2];
};

QuadraticResult solveQuadratic(double a, double b, double c)
{
    double d = b * b - 4.0 * a * c;
    if (d > 0.0)
    {
        QuadraticResult result{2, {0.0, 0.0}};
        // Improve precision by finding root with greatest magnitude via addition or subtraction
        if (-b < 0.0)
        {
            result.roots[0] = (-b - sqrt(d)) / (2.0 * a);
        }
        else
        {
            result.roots[0] = (-b + sqrt(d)) / (2.0 * a);
        }
        result.roots[1] = c / result.roots[0];
        return result;
    }
    else if (d == 0.0)
    {
        return {1, {-b / (2.0 * a), 0.0}};
    }
    return {0, {0.0, 0.0}};
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
    const QuadraticResult roots = solveQuadratic(A, B, C);

    if (roots.numRoots > 0)
    {
        // two roots (line passes through sphere twice)
        // find the closer of the two.

        //seg; pick closest:
        if (roots.numRoots == 1 || fabs(roots.roots[0]) < fabs(roots.roots[1]))
        {
            v = d * roots.roots[0];
        }
        else
        {
            v = d * roots.roots[1];
        }
    }
    dist2 = dot(v, v); // v.length2();

    if (dist2 > 0.0)
    {
        return (p0 + v) * _unitSphereToEllipsoid;
    }
    // either no intersection, or the distance was not the max.
    return {};
}

vsg::ref_ptr<vsg::Node> CsGeospatialServices::getWorldNode()
{
    return _worldNode;
}

vsg::dvec3 CsGeospatialServices::toCartographic(const vsg::dvec3 &worldPos)
{
    auto csResult = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(vsgCs::vsg2glm(worldPos));
    if (csResult)
    {
        return vsg::dvec3(csResult->longitude, csResult->latitude, csResult->height);
    }
    return vsg::dvec3(0, 0, 0); // XXX Real error result
}

vsg::dvec3 CsGeospatialServices::toWorld(const vsg::dvec3& cartographic)
{
    CesiumGeospatial::Cartographic carto(cartographic.x, cartographic.y, cartographic.z);
    glm::dvec3 csResult = CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(carto);
    return glm2vsg(csResult);
}
