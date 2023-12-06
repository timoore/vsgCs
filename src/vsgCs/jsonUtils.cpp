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

#include "jsonUtils.h"

#include <exception>
#include <CesiumUtility/JsonHelpers.h>

using namespace vsgCs;

vsg::ref_ptr<vsg::Object> JSONObjectFactory::build(const rapidjson::Value& value,
                                                   const std::string& typeOverride,
                                                   const vsg::ref_ptr<vsg::Object>& object)
{
    std::string jsonType;
    if (!typeOverride.empty())
    {
        jsonType = typeOverride;
    }
    else
    {
        jsonType = CesiumUtility::JsonHelpers::getStringOrDefault(value, "Type", "");
    }
    if (jsonType.empty())
    {
        throw std::runtime_error("No Type in JSON");
    }
    Builder builder = nullptr;
    if (auto itr = builders.find(jsonType); itr != builders.end())
    {
        builder = itr->second;
    }
    else
    {
        throw std::runtime_error("No builder for Type " + jsonType);
    }
    return (*builder)(value, this, object);
}

vsg::ref_ptr<JSONObjectFactory> JSONObjectFactory::get()
{
    static auto factory = JSONObjectFactory::create();
    return factory;
}

std::string vsgCs::getStringOrError(const rapidjson::Value& json, const std::string& key,
                                    const char* errorMsg)
{
    const auto it = json.FindMember(key.c_str());
    if (it != json.MemberEnd() && it->value.IsString())
    {
        return it->value.GetString();
    }
    throw std::runtime_error(errorMsg);
}

// XXX Very gross workaround to static library problems...

namespace vsgCs
{
    vsg::ref_ptr<vsg::Object> buildCsDebugColorizeTilesOverlay(const rapidjson::Value&,
                                                               JSONObjectFactory*,
                                                               const vsg::ref_ptr<vsg::Object>& object);
    JSONObjectFactory::Registrar r("DebugColorizeTilesRasterOverlay", buildCsDebugColorizeTilesOverlay);
    vsg::ref_ptr<vsg::Object> buildStyling(const rapidjson::Value& json,
                                           JSONObjectFactory*,
                                           const vsg::ref_ptr<vsg::Object>&);
    JSONObjectFactory::Registrar rStyling("Styling", buildStyling);
}
