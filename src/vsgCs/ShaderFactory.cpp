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

#include "ShaderFactory.h"

#include "pbr.h"

using namespace vsgCs;

ShaderFactory::ShaderFactory(const vsg::ref_ptr<vsg::Options>& vsgOptions)
    : _vsgOptions(vsgOptions) 
{
}

vsg::ref_ptr<vsg::ShaderSet> ShaderFactory::getShaderSet(ShaderDomain domain, VkPrimitiveTopology topology)
{
    vsg::ref_ptr<vsg::ShaderSet> result;
    const auto key = std::make_pair(domain, topology);
    auto itr = _shaderSetMap.find(key);
    if (itr == _shaderSetMap.end())
    {
        if (domain == MODEL)
        {
            // XXX No point version yet
            result = pbr::makeModelShaderSet(_vsgOptions);
        }
        else
        {
            result = (topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST
                      ? pbr::makePointShaderSet(_vsgOptions)
                      : pbr::makeShaderSet(_vsgOptions));
        }
        _shaderSetMap.insert({key, result});
    }
    else
    {
        result = itr->second;
    }
    return result;
}
