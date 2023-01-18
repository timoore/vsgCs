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

// Extended DescriptorConfigurator that assignes a set number to the descriptor set and checks that
// texture uniform bindings are consistent with it.

#include <optional>

#include <vsg/utils/GraphicsPipelineConfigurator.h>

namespace vsgCs {
    class DescriptorSetConfigurator : public vsg::Inherit<vsg::DescriptorConfigurator, class DescriptorSetConfigurator>
    {
      public:
        /** @brief Create a configurator for a specific set of a shader set.
         */
        DescriptorSetConfigurator(uint32_t in_setNumber, vsg::ref_ptr<vsg::ShaderSet> in_shaderSet = {})
            : Inherit(in_shaderSet), setNumber(in_setNumber)
        {}
        /** @brief Create a configurator for with no assigned set number.
         *
         * The set number will be initialized from the first assignment.
         */
        DescriptorSetConfigurator(vsg::ref_ptr<vsg::ShaderSet> in_shaderSet = {})
            : Inherit(in_shaderSet)
        {

        }
        /** @brief Assign texture and verify set number.
         */
        bool assignTexture(const std::string& name, vsg::ref_ptr<vsg::Data> textureData = {},
                           vsg::ref_ptr<vsg::Sampler> sampler = {});

        /** @brief Assign textures and verify set number.
         */
        bool assignTexture(const std::string& name, const vsg::ImageInfoList& imageInfoList, uint32_t arrayElement = 0);

        /** @brief Assign uniform buffer and verify set number.
         */
        bool assignUniform(const std::string& name, vsg::ref_ptr<vsg::Data> data = {});

        /** @brief Assign uniform buffers and verify set number.
         */
        bool assignUniform(const std::string& name, const vsg::BufferInfoList& bufferInfoList, uint32_t arrayElement = 0);
        std::optional<uint32_t> setNumber;
    };
}
