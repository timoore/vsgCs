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

#include <map>

using namespace vsgCs;

GeoNode::GeoNode(const std::string& crs)
{
    _crs = std::make_shared<CRS>(crs);
}

void GeoNode::setOrigin(const vsg::dvec3& origin)
{
    matrix = _crs->getENU(origin);
}

// Try stepping back from runtime environment singleton by passing the environment as an argument.

// Models loaded via vsgXchange are placed in their own graph, with a different arrangement of
// descriptor sets bound via state groups. Only the view / lighting / shadow descriptor set is
// bound; the world and tile parameters are of course not relevent.

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
                                           JSONObjectFactory*,
                                           const vsg::ref_ptr<vsg::Object>&)
    {
        std::map<std::string, vsg::ref_ptr<vsg::Node>> models;
        using namespace CesiumGeospatial;
        std::string crs = CesiumUtility::JsonHelpers::getStringOrDefault(json, "crs", "wgs84");
        // Geographic coordinates are in degrees. If they are supplied, then we should probably
        // check that the CRS is geographic / geodetic.
        auto earthCoords = CesiumUtility::JsonHelpers::getDoubles(json, 3, "geographicCoordinates");
        auto coordinates = CesiumUtility::JsonHelpers::getDoubles(json, 3, "coordinates");
        auto heading = CesiumUtility::JsonHelpers::getDoubleOrDefault(json, "heading", 0.0);
        vsg::dvec3 crsCoords;
        if (earthCoords)
        {
            crsCoords.set(vsg::radians((*earthCoords)[0]),
                          vsg::radians((*earthCoords)[1]),
                          (*earthCoords)[2]);
        }
        else if (coordinates)
        {
            crsCoords.set((*coordinates)[0], (*coordinates)[1], (*coordinates)[2]);
        }
        else
        {
            vsg::error("GeoNode JSON has No valid coordinates");
            return {};
        }
        auto geoNode = GeoNode::create(crs);
        geoNode->setOrigin(crsCoords);
        auto node = vsg::MatrixTransform::create();
        node->matrix =  vsg::rotate(vsg::radians(-heading), 0.0, 0.0, 1.0);
        geoNode->addChild(node);

        auto modelDefsItr = json.FindMember("modelDefinitions");
        if (modelDefsItr != json.MemberEnd())
        {
            const auto& modelDefs = modelDefsItr->value;
            if (modelDefs.IsArray())
            {
                for (rapidjson::SizeType i = 0; i < modelDefs.Size(); ++i)
                {
                    const rapidjson::Value& modelDef = modelDefs[i].GetObject();
                    auto name = getStringOrError(modelDef, "name", "Missing name in model definition");
                    // An empty path isn't an error; the user might not want a model for some reason.
                    auto path = CesiumUtility::JsonHelpers::getStringOrDefault(modelDef, "path", "");
                    // options is initialized for using the basic vsgCs shader with vsgXchange
                    // models, including the option to treat textures as sRGB.
                    if (!path.empty())
                    {
                        auto modelNode = vsg::read_cast<vsg::Node>(path, RuntimeEnvironment::get()->options);
                        if (! modelNode)
                        {
                            vsg::error("Couldn't read ", path);
                        }
                        else
                        {
                            models[name] = modelNode;
                        }
                    }
                }
            }
        }
        auto instancesItr = json.FindMember("instances");
        if (instancesItr != json.MemberEnd())
        {
            const auto& instances = instancesItr->value;
            if (instances.IsArray())
            {
                for (rapidjson::SizeType i = 0; i < instances.Size(); ++i)
                {
                    const rapidjson::Value& instance = instances[i].GetObject();
                    auto modelName = CesiumUtility::JsonHelpers::getStringOrDefault(instance, "modelName",
                                                                                    "");
                    auto coords = CesiumUtility::JsonHelpers::getDoubles(instance, 3, "coordinates");
                    auto modelHeading = CesiumUtility::JsonHelpers::getDoubleOrDefault(instance, "heading",
                                                                                       0.0);
                    auto scale  = CesiumUtility::JsonHelpers::getDoubleOrDefault(instance, "scale", 1.0);
                    vsg::dmat4 modelMat = vsg::rotate(vsg::radians(-modelHeading), 0.0, 0.0, 1.0)
                        * vsg::scale(scale, scale, scale);
                    if (coords)
                    {
                        modelMat = vsg::translate((*coords)[0], (*coords)[1], (*coords)[2]) * modelMat;
                    }
                    auto modelTransform = vsg::MatrixTransform::create(modelMat);
                    if (!modelName.empty())
                    {
                        if (auto defItr = models.find(modelName); defItr != models.end())
                        {
                            modelTransform->addChild(defItr->second);
                        }
                    }
                    node->addChild(modelTransform);
                }
            }
        }
        return geoNode;
    }

    JSONObjectFactory::Registrar r("GeoNode", buildGeoNode);
}
