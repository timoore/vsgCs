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

#include "Export.h"

#include <vsg/core/Inherit.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat3.h>

#include <optional>

namespace vsgCs
{
    class IGeospatialServices : public vsg::Inherit<vsg::Object, IGeospatialServices>
    {
    public:
        virtual vsg::dmat4 localToWorldMatrix(const vsg::dvec3& worldPos) = 0;
        virtual bool isGeocentric() = 0;
        virtual double semiMajorAxis() = 0;
        virtual vsg::dbox bounds() = 0;
        virtual std::optional<vsg::dvec3>
        intersectGeocentricLine(const vsg::dvec3& pnt1, const vsg::dvec3& pnt2) = 0;
        virtual vsg::ref_ptr<vsg::Node> getWorldNode() = 0;
        // Cartographic coordinates in radians / meters: long, lat, height
        virtual vsg::dvec3 toCartographic(const vsg::dvec3& worldPos) = 0;
        virtual vsg::dvec3 toWorld(const vsg::dvec3& cartographic) = 0;
    };

    class CsGeospatialServices : public vsg::Inherit<IGeospatialServices, CsGeospatialServices>
    {
    public:
        CsGeospatialServices(const vsg::ref_ptr<vsg::Node>& worldNode);
        vsg::dmat4 localToWorldMatrix(const vsg::dvec3& worldPos) override;
        bool isGeocentric() override;
        double semiMajorAxis() override;
        vsg::dbox bounds() override;
        std::optional<vsg::dvec3> intersectGeocentricLine(const vsg::dvec3& p0_world,
                                                          const vsg::dvec3& p1_world) override;
        vsg::ref_ptr<vsg::Node> getWorldNode() override;
        vsg::dvec3 toCartographic(const vsg::dvec3& worldPos) override;
        vsg::dvec3 toWorld(const vsg::dvec3& cartographic) override;

    protected:
        vsg::ref_ptr<vsg::Node> _worldNode;
        vsg::dmat3 _ellipsoidToUnitSphere;
        vsg::dmat3 _unitSphereToEllipsoid;
    };
}
