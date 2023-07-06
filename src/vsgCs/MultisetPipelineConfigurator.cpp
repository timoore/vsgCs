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

#include "MultisetPipelineConfigurator.h"
#include <map>

namespace vsgCs
{
    vsg::ref_ptr<vsg::PipelineLayout> makePipelineLayout(vsg::ref_ptr<vsg::ShaderSet> shaderSet,
                                                         const std::set<std::string>& defines, int max_set)
    {
        const auto& uniformBindings = shaderSet->uniformBindings;
        uint32_t umax_set;
        if (max_set >= 0)
        {
            umax_set = static_cast<uint32_t>(max_set);
        }
        else
        {
            auto maxItr = std::max_element(uniformBindings.begin(), uniformBindings.end(),
                                           [](const auto& val1, const auto& val2)
                                           {
                                               return val1.set < val2.set;
                                           });
            umax_set = maxItr->set;
        }
        std::map<uint32_t, vsg::DescriptorSetLayoutBindings> sets;
        for (auto& binding : uniformBindings)
        {
            if (max_set < 0 || static_cast<uint32_t>(max_set) >= binding.set)
            {
                if (binding.define.empty() || defines.find(binding.define) != defines.end())
                {
                    sets[binding.set].push_back(VkDescriptorSetLayoutBinding{binding.binding, binding.descriptorType,
                                                                             binding.descriptorCount, binding.stageFlags,
                                                                             nullptr});
                }
            }
        }
        vsg::DescriptorSetLayouts descriptorSetLayouts;
        for (uint32_t set = 0; set <= umax_set; ++set)
        {
            auto dslbItr = sets.find(set);
            if (dslbItr != sets.end())
            {
                descriptorSetLayouts.push_back(vsg::DescriptorSetLayout::create(dslbItr->second));
            }
            else
            {
                descriptorSetLayouts.push_back(vsg::DescriptorSetLayout::create());
            }
        }
        vsg::PushConstantRanges pushConstantRanges;
        for (auto& pcb : shaderSet->pushConstantRanges)
        {
            if (pcb.define.empty()) pushConstantRanges.push_back(pcb.range);
        }

        auto pipelineLayout = vsg::PipelineLayout::create(descriptorSetLayouts, pushConstantRanges);
        return pipelineLayout;
    }
}

using namespace vsgCs;

MultisetPipelineConfigurator::MultisetPipelineConfigurator(vsg::ref_ptr<vsg::ShaderSet> in_shaderSet)
    : Inherit(in_shaderSet)
{
}

void MultisetPipelineConfigurator::init()
{
    Inherit::init();
    // Now redo the layout creation
    layout = makePipelineLayout(shaderSet, defines());
    graphicsPipeline = vsg::GraphicsPipeline::create(layout, graphicsPipeline->stages, graphicsPipeline->pipelineStates,
                                                     graphicsPipeline->subpass);
    bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);
}
