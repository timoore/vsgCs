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

#include <vsg/all.h>

#include "DescriptorSetConfigurator.h"

using namespace vsgCs;

bool DescriptorSetConfigurator::assignTexture(const std::string& name, vsg::ref_ptr<vsg::Data> textureData,
                                              vsg::ref_ptr<vsg::Sampler> sampler)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        if (!setNumber)
        {
            setNumber = textureBinding.set;
        }
        if (textureBinding.set == setNumber.value())
        {
            return Inherit::assignTexture(name, textureData, sampler);
        }
        vsg::warn("Attempted binding texture ", name, " with set number ", textureBinding.set, " in set ",
                  setNumber.value());
        return false;
    }
    return false;
}

bool DescriptorSetConfigurator::assignUniform(const std::string& name, vsg::ref_ptr<vsg::Data> data)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        if (!setNumber)
        {
            setNumber = uniformBinding.set;
        }
        if (uniformBinding.set == setNumber.value())
        {
            return Inherit::assignUniform(name, data);
        }

        vsg::warn("Attempted binding uniform ", name, " with set number ", uniformBinding.set, " in set ",
                  setNumber.value());
        return false;

    }
    return false;
}

bool DescriptorSetConfigurator::assignTexture(const std::string& name, const vsg::ImageInfoList& imageInfoList,
                                              uint32_t arrayElement)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        // set up bindings
        if (!textureBinding.define.empty()) defines.insert(textureBinding.define);
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{textureBinding.binding, textureBinding.descriptorType,
                                                                  textureBinding.descriptorCount, textureBinding.stageFlags,
                                                                  nullptr});

        // create texture image and associated DescriptorSets and binding
        auto texture = vsg::DescriptorImage::create(imageInfoList, textureBinding.binding, arrayElement,
                                                    textureBinding.descriptorType);
        descriptors.push_back(texture);
        return true;
    }
    return false;
}

bool DescriptorSetConfigurator::assignUniform(const std::string& name, const vsg::BufferInfoList& bufferInfoList,
                                              uint32_t arrayElement)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        // set up bindings
        if (!uniformBinding.define.empty()) defines.insert(uniformBinding.define);
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags, nullptr});

        auto uniform = vsg::DescriptorBuffer::create(bufferInfoList, uniformBinding.binding, arrayElement);
        descriptors.push_back(uniform);

        return true;
    }
    return false;
}
