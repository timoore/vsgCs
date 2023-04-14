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

#include "GraphicsEnvironment.h"
#include "MultisetPipelineConfigurator.h"
#include "pbr.h"
#include "runtimeSupport.h"

using namespace vsgCs;

namespace
{
    vsg::ref_ptr<vsg::ImageInfo> makeDefaultTexture()
    {
        vsg::ubvec4 pixel(255, 255, 255, 255); // NOLINT: it's white
        vsg::Data::Properties props;
        props.format = VK_FORMAT_R8G8B8A8_SRGB;
        props.maxNumMipmaps = 1;
        auto arrayData = vsg::ubvec4Array2D::create(1, 1, &pixel, props);
        auto sampler = makeSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_NEAREST, VK_FILTER_NEAREST, 1);
        return vsg::ImageInfo::create(sampler, arrayData);
    }
}

GraphicsEnvironment::GraphicsEnvironment(const vsg::ref_ptr<vsg::Options> &vsgOptions,
                                         const DeviceFeatures& in_features,
                                         const vsg::ref_ptr<vsg::Device>& in_device)
    : shaderFactory(ShaderFactory::create(vsgOptions)), features(in_features),
      sharedObjects(create_or<vsg::SharedObjects>(vsgOptions->sharedObjects)),
      device(in_device),
      defaultTexture(makeDefaultTexture())

{
    std::set<std::string> shaderDefines;
    shaderDefines.insert("VSG_TWO_SIDED_LIGHTING");
    shaderDefines.insert("VSGCS_OVERLAY_MAPS");
    overlayPipelineLayout
        = makePipelineLayout(shaderFactory->getShaderSet(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
                             shaderDefines,
                             pbr::TILE_DESCRIPTOR_SET);
    miniCompileTraversal = vsg::CompileTraversal::create(device);
}

// Copied from vsg::CompileManager

vsg::CompileResult GraphicsEnvironment::miniCompile(vsg::ref_ptr<vsg::Object> object)
{
    vsg::CollectResourceRequirements collectRequirements;
    object->accept(collectRequirements);

    auto& requirements = collectRequirements.requirements;
    auto& binStack = requirements.binStack;

    vsg::CompileResult result;
    result.maxSlot = requirements.maxSlot;
    result.containsPagedLOD = requirements.containsPagedLOD;
    result.views = requirements.views;
    result.earlyDynamicData = requirements.earlyDynamicData;
    result.lateDynamicData = requirements.lateDynamicData;

    auto run_compile_traversal = [&]() -> void {
        for (auto& context : miniCompileTraversal->contexts)
        {
            vsg::ref_ptr<vsg::View> view = context->view;
            if (view && !binStack.empty())
            {
                if (auto itr = result.views.find(view.get()); itr == result.views.end())
                {
                    result.views[view] = binStack.top();
                }
            }

            context->reserve(requirements);
        }

        object->accept(*miniCompileTraversal);

        //debug("Finished compile traversal ", object);

        if (miniCompileTraversal->record())
        {
            // This happens on startup. I think it's caused by the default texture.
            vsg::warn("miniCompile did recording!");
        }

        miniCompileTraversal->waitForCompletion();

        debug("Finished waiting for compile ", object);
    };

    run_compile_traversal();

    result.result = VK_SUCCESS;
    return result;

}
