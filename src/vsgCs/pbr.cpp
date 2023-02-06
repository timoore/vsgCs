/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include "pbr.h"
#include "runtimeSupport.h"
#include "ViewDependentStateExt.h"

#include <vsg/core/Array.h>
#include <vsg/core/Array2D.h>
#include <vsg/io/Logger.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/TessellationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/material.h>
#include <vsg/utils/ShaderSet.h>
#include <vulkan/vulkan_core.h>

namespace vsgCs {
    namespace pbr
    {
        vsg::ref_ptr<vsg::Data> makeTileData(float geometricError, float maxPointSize,
                                             const gsl::span<OverlayParams> overlayUniformMem)
        {
            vsg::vec4 tileScratch;
            tileScratch[0] = geometricError;
            tileScratch[1] = maxPointSize;
            auto result = vsg::ubyteArray::create(sizeof(vsg::vec4) + overlayUniformMem.size_bytes());
            memcpy(&(*result)[0], &tileScratch, sizeof(tileScratch));
            memcpy(&(*result)[sizeof(tileScratch)], &overlayUniformMem[0], overlayUniformMem.size_bytes());
            return result;
        }

        vsg::ref_ptr<vsg::ShaderSet> makeShaderSetAux(vsg::ref_ptr<vsg::ShaderSet> shaderSet)
        {
            shaderSet->addAttributeBinding("vsg_Vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
            shaderSet->addAttributeBinding("vsg_Normal", "", 1, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));

            shaderSet->addAttributeBinding("vsg_Color", "", 2, VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));
            shaderSet->addAttributeBinding("vsg_position", "VSG_INSTANCE_POSITIONS", 3, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
            shaderSet->addAttributeBinding("vsg_TexCoord0", "", 4, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
            shaderSet->addAttributeBinding("vsg_TexCoord1", "", 5, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
            shaderSet->addAttributeBinding("vsg_TexCoord2", "", 6, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
            shaderSet->addAttributeBinding("vsg_TexCoord3", "", 7, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
            shaderSet->addUniformBinding("displacementMap", "VSG_DISPLACEMENT_MAP", PRIMITIVE_DESCRIPTOR_SET, 6,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, vsg::vec4Array2D::create(1, 1));
            shaderSet->addUniformBinding("diffuseMap", "VSG_DIFFUSE_MAP", PRIMITIVE_DESCRIPTOR_SET, 0,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1, 1));
            shaderSet->addUniformBinding("mrMap", "VSG_METALLROUGHNESS_MAP", PRIMITIVE_DESCRIPTOR_SET, 1,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1, 1));
            shaderSet->addUniformBinding("normalMap", "VSG_NORMAL_MAP", PRIMITIVE_DESCRIPTOR_SET, 2,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec3Array2D::create(1, 1));
            shaderSet->addUniformBinding("aoMap", "VSG_LIGHTMAP_MAP", PRIMITIVE_DESCRIPTOR_SET, 3,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1, 1));
            shaderSet->addUniformBinding("emissiveMap", "VSG_EMISSIVE_MAP", PRIMITIVE_DESCRIPTOR_SET, 4,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1, 1));
            shaderSet->addUniformBinding("specularMap", "VSG_SPECULAR_MAP", PRIMITIVE_DESCRIPTOR_SET, 5,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1, 1));
            shaderSet->addUniformBinding("material", "", PRIMITIVE_DESCRIPTOR_SET, 10,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::PbrMaterialValue::create());
            shaderSet->addUniformBinding("lightData", "", VIEW_DESCRIPTOR_SET, 0,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));
            shaderSet->addUniformBinding("viewData", "", VIEW_DESCRIPTOR_SET, 1,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 ,VK_SHADER_STAGE_VERTEX_BIT, vsg::ubyteArray::create(sizeof(viewport)));
            shaderSet->addUniformBinding("tileParams", "", TILE_DESCRIPTOR_SET, 0,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(1 + sizeof(OverlayParams)));
            shaderSet->addUniformBinding("overlayTextures", "", TILE_DESCRIPTOR_SET, 1,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxOverlays, VK_SHADER_STAGE_FRAGMENT_BIT, {});
            // additional defines
            shaderSet->optionalDefines = {"VSG_GREYSACLE_DIFFUSE_MAP", "VSG_TWO_SIDED_LIGHTING", "VSG_WORKFLOW_SPECGLOSS"};

            shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

            shaderSet->definesArrayStates.push_back(vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS", "VSG_DISPLACEMENT_MAP"}, vsg::PositionAndDisplacementMapArrayState::create()});
            shaderSet->definesArrayStates.push_back(vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS"}, vsg::PositionArrayState::create()});
            shaderSet->definesArrayStates.push_back(vsg::DefinesArrayState{{"VSG_DISPLACEMENT_MAP"}, vsg::DisplacementMapArrayState::create()});

            return shaderSet;
        }

        vsg::ref_ptr<vsg::ShaderSet> makeShaderSet(vsg::ref_ptr<const vsg::Options> options)
        {
            auto vertexShader = vsg::read_cast<vsg::ShaderStage>("shaders/csstandard.vert", options);
            auto fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/csstandard_pbr.frag", options);

            if (!vertexShader || !fragmentShader)
            {
                vsg::fatal("pbr::makeShaderSet(...) could not find shaders.");
                return {};
            }
            auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

            vsg::ShaderStage::SpecializationConstants specializationConstats{
                {0, vsg::intValue::create(maxOverlays)}, // numTiles
            };

            makeShaderSetAux(shaderSet);
            shaderSet->optionalDefines.insert({"VSGCS_FLAT_SHADING", "VSGCS_BILLBOARD_NORMAL"});
            return shaderSet;
        }

        vsg::ref_ptr<vsg::ShaderSet> makePointShaderSet(vsg::ref_ptr<const vsg::Options> options)
        {
            auto vertexShader = vsg::read_cast<vsg::ShaderStage>("shaders/cspoint.vert", options);
            auto fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/csstandard_pbr.frag", options);

            if (!vertexShader || !fragmentShader)
            {
                vsg::fatal("pbr::makeShaderSet(...) could not find shaders.");
                return {};
            }
            auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

            vsg::ShaderStage::SpecializationConstants specializationConstats{
                {0, vsg::intValue::create(maxOverlays)}, // numTiles
            };

            makeShaderSetAux(shaderSet);
            shaderSet->optionalDefines.insert({"VSGCS_BILLBOARD_NORMAL", "VSGCS_SIZE_TO_ERROR"});
            return shaderSet;
        }
    }
}

