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

#include "Styling.h"

#include "jsonUtils.h"

#include <vsg/maths/vec4.h>

#include <CesiumUtility/JsonHelpers.h>

#include <algorithm>
#include <stdexcept>
#include <strings.h>
#include <map>
#include <string>

using namespace vsgCs;

Styling::Styling() : show(true) {}

Styling::Styling(bool in_show)
    : show(in_show)
{
}

Styling::Styling(bool in_show, const vsg::vec4& in_color)
    : show(in_show), color(in_color)
{
}

struct CaseInsensitiveComparator
{
    bool operator()(const std::string& a, const std::string& b) const noexcept
    {
        return ::strcasecmp(a.c_str(), b.c_str()) < 0;
    }
};

template <typename T>
using CaseInsensitiveMap = std::map<std::string, T, CaseInsensitiveComparator>;

vsg::vec4 color(uint32_t r, uint32_t g, uint32_t b)
{
    return vsg::vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

const CaseInsensitiveMap<vsg::vec4> colors = {
    {"aliceblue", color(0xF0, 0xF8, 0xFF)},
    {"antiquewhite", color(0xFA, 0xEB, 0xD7)},
    {"aqua", color(0x00, 0xFF, 0xFF)},
    {"aquamarine", color(0x7F, 0xFF, 0xD4)},
    {"azure", color(0xF0, 0xFF, 0xFF)},
    {"beige", color(0xF5, 0xF5, 0xDC)},
    {"bisque", color(0xFF, 0xE4, 0xC4)},
    {"black", color(0x00, 0x00, 0x00)},
    {"blanchedalmond", color(0xFF, 0xEB, 0xCD)},
    {"blue", color(0x00, 0x00, 0xFF)},
    {"blueviolet", color(0x8A, 0x2B, 0xE2)},
    {"brown", color(0xA5, 0x2A, 0x2A)},
    {"burlywood", color(0xDE, 0xB8, 0x87)},
    {"cadetblue", color(0x5F, 0x9E, 0xA0)},
    {"chartreuse", color(0x7F, 0xFF, 0x00)},
    {"chocolate", color(0xD2, 0x69, 0x1E)},
    {"coral", color(0xFF, 0x7F, 0x50)},
    {"cornflowerblue", color(0x64, 0x95, 0xED)},
    {"cornsilk", color(0xFF, 0xF8, 0xDC)},
    {"crimson", color(0xDC, 0x14, 0x3C)},
    {"cyan", color(0x00, 0xFF, 0xFF)},
    {"darkblue", color(0x00, 0x00, 0x8B)},
    {"darkcyan", color(0x00, 0x8B, 0x8B)},
    {"darkgoldenrod", color(0xB8, 0x86, 0x0B)},
    {"darkgray", color(0xA9, 0xA9, 0xA9)},
    {"darkgreen", color(0x00, 0x64, 0x00)},
    {"darkgrey", color(0xA9, 0xA9, 0xA9)},
    {"darkkhaki", color(0xBD, 0xB7, 0x6B)},
    {"darkmagenta", color(0x8B, 0x00, 0x8B)},
    {"darkolivegreen", color(0x55, 0x6B, 0x2F)},
    {"darkorange", color(0xFF, 0x8C, 0x00)},
    {"darkorchid", color(0x99, 0x32, 0xCC)},
    {"darkred", color(0x8B, 0x00, 0x00)},
    {"darksalmon", color(0xE9, 0x96, 0x7A)},
    {"darkseagreen", color(0x8F, 0xBC, 0x8F)},
    {"darkslateblue", color(0x48, 0x3D, 0x8B)},
    {"darkslategray", color(0x2F, 0x4F, 0x4F)},
    {"darkslategrey", color(0x2F, 0x4F, 0x4F)},
    {"darkturquoise", color(0x00, 0xCE, 0xD1)},
    {"darkviolet", color(0x94, 0x00, 0xD3)},
    {"deeppink", color(0xFF, 0x14, 0x93)},
    {"deepskyblue", color(0x00, 0xBF, 0xFF)},
    {"dimgray", color(0x69, 0x69, 0x69)},
    {"dimgrey", color(0x69, 0x69, 0x69)},
    {"dodgerblue", color(0x1E, 0x90, 0xFF)},
    {"firebrick", color(0xB2, 0x22, 0x22)},
    {"floralwhite", color(0xFF, 0xFA, 0xF0)},
    {"forestgreen", color(0x22, 0x8B, 0x22)},
    {"fuchsia", color(0xFF, 0x00, 0xFF)},
    {"gainsboro", color(0xDC, 0xDC, 0xDC)},
    {"ghostwhite", color(0xF8, 0xF8, 0xFF)},
    {"gold", color(0xFF, 0xD7, 0x00)},
    {"goldenrod", color(0xDA, 0xA5, 0x20)},
    {"gray", color(0x80, 0x80, 0x80)},
    {"green", color(0x00, 0x80, 0x00)},
    {"greenyellow", color(0xAD, 0xFF, 0x2F)},
    {"grey", color(0x80, 0x80, 0x80)},
    {"honeydew", color(0xF0, 0xFF, 0xF0)},
    {"hotpink", color(0xFF, 0x69, 0xB4)},
    {"indianred", color(0xCD, 0x5C, 0x5C)},
    {"indigo", color(0x4B, 0x00, 0x82)},
    {"ivory", color(0xFF, 0xFF, 0xF0)},
    {"khaki", color(0xF0, 0xE6, 0x8C)},
    {"lavender", color(0xE6, 0xE6, 0xFA)},
    {"lavenderblush", color(0xFF, 0xF0, 0xF5)},
    {"lawngreen", color(0x7C, 0xFC, 0x00)},
    {"lemonchiffon", color(0xFF, 0xFA, 0xCD)},
    {"lightblue", color(0xAD, 0xD8, 0xE6)},
    {"lightcoral", color(0xF0, 0x80, 0x80)},
    {"lightcyan", color(0xE0, 0xFF, 0xFF)},
    {"lightgoldenrodyellow", color(0xFA, 0xFA, 0xD2)},
    {"lightgray", color(0xD3, 0xD3, 0xD3)},
    {"lightgreen", color(0x90, 0xEE, 0x90)},
    {"lightgrey", color(0xD3, 0xD3, 0xD3)},
    {"lightpink", color(0xFF, 0xB6, 0xC1)},
    {"lightsalmon", color(0xFF, 0xA0, 0x7A)},
    {"lightseagreen", color(0x20, 0xB2, 0xAA)},
    {"lightskyblue", color(0x87, 0xCE, 0xFA)},
    {"lightslategray", color(0x77, 0x88, 0x99)},
    {"lightslategrey", color(0x77, 0x88, 0x99)},
    {"lightsteelblue", color(0xB0, 0xC4, 0xDE)},
    {"lightyellow", color(0xFF, 0xFF, 0xE0)},
    {"lime", color(0x00, 0xFF, 0x00)},
    {"limegreen", color(0x32, 0xCD, 0x32)},
    {"linen", color(0xFA, 0xF0, 0xE6)},
    {"magenta", color(0xFF, 0x00, 0xFF)},
    {"maroon", color(0x80, 0x00, 0x00)},
    {"mediumaquamarine", color(0x66, 0xCD, 0xAA)},
    {"mediumblue", color(0x00, 0x00, 0xCD)},
    {"mediumorchid", color(0xBA, 0x55, 0xD3)},
    {"mediumpurple", color(0x93, 0x70, 0xDB)},
    {"mediumseagreen", color(0x3C, 0xB3, 0x71)},
    {"mediumslateblue", color(0x7B, 0x68, 0xEE)},
    {"mediumspringgreen", color(0x00, 0xFA, 0x9A)},
    {"mediumturquoise", color(0x48, 0xD1, 0xCC)},
    {"mediumvioletred", color(0xC7, 0x15, 0x85)},
    {"midnightblue", color(0x19, 0x19, 0x70)},
    {"mintcream", color(0xF5, 0xFF, 0xFA)},
    {"mistyrose", color(0xFF, 0xE4, 0xE1)},
    {"moccasin", color(0xFF, 0xE4, 0xB5)},
    {"navajowhite", color(0xFF, 0xDE, 0xAD)},
    {"navy", color(0x00, 0x00, 0x80)},
    {"oldlace", color(0xFD, 0xF5, 0xE6)},
    {"olive", color(0x80, 0x80, 0x00)},
    {"olivedrab", color(0x6B, 0x8E, 0x23)},
    {"orange", color(0xFF, 0xA5, 0x00)},
    {"orangered", color(0xFF, 0x45, 0x00)},
    {"orchid", color(0xDA, 0x70, 0xD6)},
    {"palegoldenrod", color(0xEE, 0xE8, 0xAA)},
    {"palegreen", color(0x98, 0xFD, 0x98)},
    {"paleturquoise", color(0xAF, 0xEE, 0xEE)},
    {"palevioletred", color(0xDB, 0x70, 0x93)},
    {"papayawhip", color(0xFF, 0xEF, 0xD5)},
    {"peachpuff", color(0xFF, 0xDA, 0xB9)},
    {"peru", color(0xCD, 0x85, 0x3F)},
    {"pink", color(0xFF, 0xC0, 0xCD)},
    {"plum", color(0xDD, 0xA0, 0xDD)},
    {"powderblue", color(0xB0, 0xE0, 0xE6)},
    {"purple", color(0x80, 0x00, 0x80)},
    {"red", color(0xFF, 0x00, 0x00)},
    {"rosybrown", color(0xBC, 0x8F, 0x8F)},
    {"royalblue", color(0x41, 0x69, 0xE1)},
    {"saddlebrown", color(0x8B, 0x45, 0x13)},
    {"salmon", color(0xFA, 0x80, 0x72)},
    {"sandybrown", color(0xF4, 0xA4, 0x60)},
    {"seagreen", color(0x2E, 0x8B, 0x57)},
    {"seashell", color(0xFF, 0xF5, 0xEE)},
    {"sienna", color(0xA0, 0x52, 0x2D)},
    {"silver", color(0xC0, 0xC0, 0xC0)},
    {"skyblue", color(0x87, 0xCE, 0xEB)},
    {"slateblue", color(0x6A, 0x5A, 0xCD)},
    {"slategray", color(0x70, 0x80, 0x90)},
    {"slategrey", color(0x70, 0x80, 0x90)},
    {"snow", color(0xFF, 0xFA, 0xFA)},
    {"springgreen", color(0x00, 0xFF, 0x7F)},
    {"steelblue", color(0x46, 0x82, 0xB4)},
    {"tan", color(0xD2, 0xB4, 0x8C)},
    {"teal", color(0x00, 0x80, 0x80)},
    {"thistle", color(0xD8, 0xBF, 0xD8)},
    {"tomato", color(0xFF, 0x63, 0x47)},
    {"turquoise", color(0x40, 0xE0, 0xD0)},
    {"saddlebrown", color(0x8B, 0x45, 0x13)},
    {"violet", color(0xEE, 0x82, 0xEE)},
    {"wheat", color(0xF5, 0xDE, 0xB3)},
    {"white", color(0xFF, 0xFF, 0xFF)},
    {"whitesmoke", color(0xF5, 0xF5, 0xF5)},
    {"yellow", color(0xFF, 0xFF, 0x00)},
    {"yellowgreen", color(0x9A, 0xCD, 0x32)}};

void colorError(const std::string& colorSpec)
{
    throw std::runtime_error("can't parse color " + colorSpec);
}

vsg::vec4 hexColor(const std::string &color)
{
    if (color.size() != 6)
    {
        colorError(color);
    }
    unsigned long vals[3];
    for (int i = 0; i < 3; ++i)
    {
        vals[i] = std::stoul(std::string(color.begin() + 2 * i,
                                         color.begin() + 2 * (i + 1)),
                             nullptr,
                             16);
    }
    return vsg::vec4(vals[0] / 255.0f, vals[1] / 255.0f, vals[2] / 255.0f, 1.0f);
}

vsg::vec4 parseColorExpr(const std::string& expr)
{
    const vsg::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
    const std::string colorFunc("color(");
    if (colorFunc.size() > expr.size())
    {
        colorError(expr);
    }
    auto match = std::mismatch(colorFunc.begin(), colorFunc.end(), expr.begin());
    if (match.first != colorFunc.end())
    {
        colorError(expr);
    }
    if (*match.second == ')')
    {
        return white;           // color()
    }
    if (*match.second == '\'' || *match.second == '"')
    {
        auto closing = std::find(match.second + 1, expr.end(), *match.second);
        if (closing == expr.end())
        {
            colorError(expr);
        }
        std::string color(match.second + 1, closing);
        if (color[0] == '#')
        {
            return hexColor(std::string(color.begin() + 1, color.end()));
        }
        auto w3cColor = colors.find(color);
        if (w3cColor == colors.end())
        {
            colorError(color);
        }
        return w3cColor->second;
    }
    colorError(expr);
    // Shouldn't get here
    return vsg::vec4(0.0, 0.0, 0.0, 1.0);
}

namespace vsgCs
{
    vsg::ref_ptr<vsg::Object> buildStyling(const rapidjson::Value& json,
                                           JSONObjectFactory*,
                                           vsg::ref_ptr<vsg::Object>)
    {
        auto showStr = CesiumUtility::JsonHelpers::getStringOrDefault(json, "show", "true");
        bool show = showStr == "true";
        const auto colorItr = json.FindMember("color");
        if (colorItr != json.MemberEnd() && colorItr->value.IsString())
        {
            auto colorVal = parseColorExpr(colorItr->value.GetString());
            return Styling::create(show, colorVal);
        }
        return Styling::create(show);
    }
}
