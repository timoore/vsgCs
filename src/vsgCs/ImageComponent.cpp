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

#include "ImageComponent.h"

#include <vsgImGui/RenderImGui.h>

#include <vsg/all.h>

using namespace vsgCs;

namespace
{
    static vsg::DescriptorSetLayoutBindings descriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
    };

    vsg::ref_ptr<vsg::DescriptorSet> makeImageDescriptorSet(vsg::ref_ptr<vsg::Data> textureData)
    {
    if (!textureData) return {};
    // set up graphics pipeline
    auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);
    // create texture image and associated DescriptorSets and binding
    auto sampler = vsg::Sampler::create();
    sampler->maxLod = 9.0;      // whatever for now
    sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    auto texture = vsg::DescriptorImage::create(sampler, textureData, 0, 0,
                                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{texture});
    return descriptorSet;
    }

    vsg::ref_ptr<vsg::DescriptorSet> makeImageDescriptorSet(vsg::ref_ptr<vsg::ImageInfo> info)
    {
    if (!info->imageView->image->data) return {};
    // set up graphics pipeline
    auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);
    // create texture image and associated DescriptorSets and binding
    auto texture = vsg::DescriptorImage::create(info, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{texture});
    return descriptorSet;
    }
} // namespace

ImageComponent::ImageComponent(vsg::ref_ptr<vsg::Window> window,
                               vsg::ref_ptr<vsg::Data> texData)
    :  height(texData->height()), width(texData->width()), _window(window)
{
    
    descriptorSet = makeImageDescriptorSet(texData);
}

ImageComponent::ImageComponent(vsg::ref_ptr<vsg::Window> window,
                               vsg::ref_ptr<vsg::ImageInfo> info)
    : _window(window)
{
    auto data = info->imageView->image->data;
    height = data->height();
    width = data->width();
    descriptorSet = makeImageDescriptorSet(info);
}

void ImageComponent::compile(vsg::Context& context)
{
    if (_window->getDevice()->deviceID != context.deviceID)
    {
        vsg::fatal("ImageComponent can only be used in the Window for which it was created.");
        return;
    }
    descriptorSet->compile(context);
}

bool ImageComponent::operator()()
{
    ImGui::Image(static_cast<ImTextureID>(descriptorSet->vk(_window->getDevice()->deviceID)),
                 ImVec2(static_cast<float>(width), static_cast<float>(height)));
    return true;
}

void ImageComponent::record(vsg::CommandBuffer&) const
{
    vsg::fatal("An ImageComponent can't be recorded.");
}
