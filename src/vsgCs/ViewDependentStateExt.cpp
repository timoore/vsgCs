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

#include "ViewDependentStateExt.h"
#include <vsg/app/View.h>
#include <vsg/vk/Context.h>

using namespace vsgCs;

ViewDependentStateExt::ViewDependentStateExt(uint32_t maxNumberLights)
    : Inherit(maxNumberLights)
{
    vsg::Data::Properties props;
    props.dataVariance = vsg::DYNAMIC_DATA;
    viewportData = vsg::ubyteArray::create(sizeof(viewport), props);
    viewportDataBufferInfo = vsg::BufferInfo::create(viewportData.get());
    vsg::DescriptorSetLayoutBindings descriptorBindings{
        // lighting parameters
        VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        // viewport parameters
        VkDescriptorSetLayoutBinding{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
    // overwrite the base class' descriptor set layout and descriptor set
    descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);
    viewportDescriptor = vsg::DescriptorBuffer::create(vsg::BufferInfoList{viewportDataBufferInfo}, 1);
    descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout,
                                               vsg::Descriptors{descriptor, viewportDescriptor});
}

void ViewDependentStateExt::update(vsg::View& view)
{
    viewport viewportParams;
    if (view.camera)
    {
        VkViewport viewport = view.camera->getViewport();
        viewportParams.viewportExtent[0] = viewport.x;
        viewportParams.viewportExtent[1] = viewport.y;
        viewportParams.viewportExtent[2] = viewport.width;
        viewportParams.viewportExtent[3] = viewport.height;
        VkRect2D scissors = view.camera->getRenderArea();
        viewportParams.scissorExtent[0] = scissors.offset.x;
        viewportParams.scissorExtent[1] = scissors.offset.y;
        viewportParams.scissorExtent[2] = scissors.extent.width;
        viewportParams.scissorExtent[3] = scissors.extent.height;
        viewportParams.depthExtent[0] = viewport.minDepth;
        viewportParams.depthExtent[1] = viewport.maxDepth;
        viewportParams.pad[0] = 0.0f;
        viewportParams.pad[1] = 0.0f;
        memcpy(viewportData->data(), &viewportParams, sizeof(viewportParams));
        viewportData->dirty();
    }
}
