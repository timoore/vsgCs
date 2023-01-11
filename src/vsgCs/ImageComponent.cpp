#include "ImageComponent.h"

#include <vsgImGui/RenderImGui.h>

#include <vsg/all.h>

using namespace vsgCs;

namespace
{
    vsg::ref_ptr<vsg::DescriptorSet> makeImageDescriptorSet(vsg::ref_ptr<vsg::Data> texData)
    {
        // Code stolen from vsgviewer's createTextureQuad. I don't know if our own code would have
        // any advantage here.
        // We really only care about creating a 32 bit texture format if needed.
        struct ConvertToRGBA : public vsg::Visitor
    {
        vsg::ref_ptr<vsg::Data> textureData;

        void apply(vsg::Data& data) override
        {
            textureData = &data;
        }

        void apply(vsg::ubvec3Array2D& uba) override
        {
            vsg::Data::Properties newProps;
            newProps.format  = VK_FORMAT_R8G8B8A8_UNORM;
            if (uba.properties.format == VK_FORMAT_R8G8B8_SRGB)
            {
                newProps.format = VK_FORMAT_R8G8B8A8_SRGB;
            }
            else if (uba.properties.format != VK_FORMAT_R8G8B8_UNORM)
            {
                vsg::warn("Don't know what to do with format ", uba.properties.format);

            }
            auto rgba = vsg::ubvec4Array2D::create(uba.width(), uba.height(), newProps);
            auto dest_itr = rgba->begin();
            for (auto& v : uba)
            {
                (*dest_itr++).set(v.r, v.g, v.b, 255);
            }
            textureData = rgba;

        }
        vsg::ref_ptr<vsg::Data> convert(vsg::ref_ptr<vsg::Data> data)
        {
            data->accept(*this);
            return textureData;
        }

    } convertToRGBA;
        auto textureData = convertToRGBA.convert(texData);
    if (!textureData) return {};
        // set up graphics pipeline

    vsg::DescriptorSetLayoutBindings descriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
    };

    auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);
    // create texture image and associated DescriptorSets and binding
    auto sampler = vsg::Sampler::create();
    sampler->maxLod = 9.0;      // whatever for now
    sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    auto texture = vsg::DescriptorImage::create(sampler, textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

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
                 ImVec2(width, height));
    return true;
}

void ImageComponent::record(vsg::CommandBuffer&) const
{
    vsg::fatal("An ImageComponent can't be recorded.");
}
