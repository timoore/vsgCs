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
        else
        {
            vsg::warn("Attempted binding texture ", name, " with set number ", textureBinding.set, " in set ",
                      setNumber.value());
            return false;
        }

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
        else
        {
            vsg::warn("Attempted binding uniform ", name, " with set number ", uniformBinding.set, " in set ",
                      setNumber.value());
            return false;
        }
    }
    return false;
}
