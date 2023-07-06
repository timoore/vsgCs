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

#include "vsg/utils/GraphicsPipelineConfigurator.h"

namespace vsgCs {
    /**
     * @brief Create a pipeline layout for some sets of a shader set.
     *
     * @param shaderSet the shader set
     * @param defines Set of defines for assembling the bindings in the sets
     * @param sets Number of sets to use, starting from 0. Defaults to all sets.
     * @return the vsg::PipelineLayout object.
     */
    vsg::ref_ptr<vsg::PipelineLayout> makePipelineLayout(vsg::ref_ptr<vsg::ShaderSet> shaderSet,
                                                         const std::set<std::string>& defines = {},
                                                         int sets = -1);

    /**
     * @brief Subclass of vsg::GraphicsPipelineConfigurator that handles multiple descriptor sets.
     *
     * vsg::GraphicsPipelineConfigurator only handles one additional descriptor set. This class
     * ignores that and builds the pipeline layout using the sets specified by the shader set.
     */
    class MultisetPipelineConfigurator : public vsg::Inherit<vsg::GraphicsPipelineConfigurator, MultisetPipelineConfigurator>
    {
        public:
        MultisetPipelineConfigurator(vsg::ref_ptr<vsg::ShaderSet> in_shaderSet);
        bool enableTexture(const std::string& name) = delete;
        bool enableUniform(const std::string& name) = delete;
        bool assignTexture(vsg::Descriptors& descriptors, const std::string& name, vsg::ref_ptr<vsg::Data> textureData = {},
                           vsg::ref_ptr<vsg::Sampler> sampler = {}) = delete;
        bool assignUniform(vsg::Descriptors& descriptors, const std::string& name,
                           vsg::ref_ptr<vsg::Data> data = {}) = delete;
        void init();
        std::set<std::string>& defines()
        {
            return shaderHints->defines;
        }
        const std::set<std::string>& defines() const
        {
            return shaderHints->defines;
        }
    };
}
