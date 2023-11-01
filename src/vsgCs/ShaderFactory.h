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

#include <vsg/core/Inherit.h>
#include <vsg/utils/ShaderSet.h>

#include <map>


namespace vsgCs
{
    enum ShaderDomain
    {
        MODEL,                  // bog-standard glTF style model
        TILESET                 // shader for 3D Tiles
    };

    class ShaderFactory : public vsg::Inherit<vsg::Object, ShaderFactory>
    {
    public:
        ShaderFactory(const vsg::ref_ptr<vsg::Options>& vsgOptions);
        vsg::ref_ptr<vsg::ShaderSet> getShaderSet(VkPrimitiveTopology topology)
        {
            return getShaderSet(TILESET, topology);
        }
        vsg::ref_ptr<vsg::ShaderSet> getShaderSet(ShaderDomain domain, VkPrimitiveTopology topology);
    protected:
        vsg::ref_ptr<vsg::Options> _vsgOptions;
        std::map<std::pair<ShaderDomain, VkPrimitiveTopology>, vsg::ref_ptr<vsg::ShaderSet>> _shaderSetMap;
    };
}
