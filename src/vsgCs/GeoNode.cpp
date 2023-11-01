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

#include "GeoNode.h"

#include "jsonUtils.h"
#include "runtimeSupport.h"
#include "RuntimeEnvironment.h"
#include "pbr.h"

#include <CesiumUtility/JsonHelpers.h>
#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>

#include <vsg/maths/transform.h>
#include <vsg/state/ViewDependentState.h>

using namespace vsgCs;

// Try stepping back from runtime environment singleton
vsg::ref_ptr<vsg::StateGroup> vsgCs::createModelRoot(const vsg::ref_ptr<RuntimeEnvironment>& env)
{
    std::set<std::string> shaderDefines;
    shaderDefines.insert("VSG_TWO_SIDED_LIGHTING");
    // Bind the lighting for the whole world
    auto defaultShaderSet = env->genv->shaderFactory->getShaderSet(MODEL, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    auto viewPipelineLayout = defaultShaderSet->createPipelineLayout(shaderDefines,
                                                                     {0, pbr::VIEW_DESCRIPTOR_SET + 1});
    auto bindViewDescriptorSets
        = vsg::BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, viewPipelineLayout,
                                              pbr::VIEW_DESCRIPTOR_SET);
    auto result = vsg::StateGroup::create();
    result->add(bindViewDescriptorSets);
    return result;
}

namespace
{
    vsg::ref_ptr<vsg::Object> buildGeoNode(const rapidjson::Value& json,
                                           JSONObjectFactory* factory,
                                           const vsg::ref_ptr<vsg::Object>&)
    {
        using namespace CesiumGeospatial;
        auto earthCoords = CesiumUtility::JsonHelpers::getDoubles(json, 3, "coordinates");
        auto heading = CesiumUtility::JsonHelpers::getDoubleOrDefault(json, "heading", 0.0);
        auto modelName = CesiumUtility::JsonHelpers::getStringOrDefault(json, "modelName", "");
        if (!earthCoords)
        {
            vsg::error("No valid coordinates");
            return {};
        }
        if (modelName.empty())
        {
            vsg::error("No model name");
            return {};
        }
        auto node = GeoNode::create();
        LocalHorizontalCoordinateSystem localSys(Cartographic::fromDegrees((*earthCoords)[0],
                                                                           (*earthCoords)[1],
                                                                           (*earthCoords)[2]));
        vsg::dmat4 localMat = glm2vsg(localSys.getLocalToEcefTransformation());
        node->matrix = localMat * vsg::rotate(vsg::radians(-heading), 0.0, 0.0, 1.0);
        auto model = vsg::read_cast<vsg::Node>(modelName, RuntimeEnvironment::get()->options);
        if (!model)
        {
            vsg::error("Error reading model");
            return {};
        }
        node->addChild(model);
        return node;
    }

    JSONObjectFactory::Registrar r("GeoNode", buildGeoNode);
}
