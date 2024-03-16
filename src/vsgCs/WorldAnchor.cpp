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

#include "WorldAnchor.h"

#include <vsg/maths/transform.h>

namespace vsgCs
{
    WorldAnchor::WorldAnchor(const std::string& in_crs,
                             const vsg::dvec3& in_worldOrigin,
                             const vsg::dvec3& in_localOrigin)
        : crs(std::make_shared<CRS>(in_crs)), worldOrigin(in_worldOrigin), localOrigin(in_localOrigin)
    {
        enu = crs->getENU(worldOrigin);
        matrix = translate(localOrigin) * inverse(enu);
    }
}
