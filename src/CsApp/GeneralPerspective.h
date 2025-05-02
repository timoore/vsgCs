/* <editor-fold desc="MIT License">

Copyright(c) 2025 Timothy Moore

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

#include <vsg/app/ProjectionMatrix.h>

namespace CsApp
{
    struct GeneralPerspective : public vsg::Inherit<vsg::ProjectionMatrix, GeneralPerspective>
    {
        GeneralPerspective(double in_left, double in_right, double in_bottom, double in_top,
                           double in_zNear, double in_zFar)
            : left(in_left), right(in_right), bottom(in_bottom), top(in_top),
              zNear(in_zNear), zFar(in_zFar) {}

        GeneralPerspective()
            : GeneralPerspective(-1.0, 1.0, -1.0, 1.0, 1.0, 10000.0) {}

        GeneralPerspective(const GeneralPerspective& p, const vsg::CopyOp& copyop = {})
            : Inherit(p, copyop),
              left(p.left),
              right(p.right),
              bottom(p.bottom),
              top(p.top),
              zNear(p.zNear),
              zFar(p.zFar) {}

        vsg::ref_ptr<vsg::Object> clone(const vsg::CopyOp& copyop) const override
        {
            return create(*this, copyop);
        }

        vsg::dmat4 transform() const override
        {
            return vsg::perspective(left, right, bottom, top, zNear, zFar);
        }
        
        void changeExtent(const VkExtent2D& prevExtent, const VkExtent2D& newExtent) override
        {
            double oldRatio = static_cast<double>(prevExtent.width) / prevExtent.height;
            double newRatio = static_cast<double>(newExtent.width) / newExtent.height;
            double delta = newRatio / oldRatio;
            left *= delta;
            right *= delta;
            zNear *= delta;
        }

        double left;
        double right;
        double bottom;
        double top;
        double zNear;
        double zFar;
    };
}
