#pragma once

#include "vsg/utils/GraphicsPipelineConfigurator.h"

namespace vsgCs {
    vsg::ref_ptr<vsg::PipelineLayout> makePipelineLayout(vsg::ref_ptr<vsg::ShaderSet> shaderSet);
    vsg::ref_ptr<vsg::PipelineLayout> makePipelineLayout(vsg::ref_ptr<vsg::ShaderSet> shaderSet,
                                                         const std::set<std::string>& defines, int sets = -1);

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
        void init(std::set<std::string> defines);
    };
}
