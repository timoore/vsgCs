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

#include <rapidjson/document.h>
#include <vsg/io/Logger.h>

namespace vsgCs
{
    // This was written with a parameter TDest so that the definition of rapidjson::Document
    // wouldn't have to be visible, but in practice any user of this function will need to include
    // rapidjson/document.h.

    /**
     * @brief Initialize an object from raw data that can be parsed into JSON.
     *
     * @param obj An object that defines an init(const rapidjson::Value&) member function.
     */
    template<typename TObj, typename TSource, typename TDest = rapidjson::Document>
    void initFromJSON(TObj& obj, TSource&& source)
    {
        TDest document;
        document.Parse(source.data(), source.size());
        if (document.HasParseError())
        {
            vsg::error("Error parsing json: error code ", document.GetParseError(),
                       " at byte ", document.GetErrorOffset(),
                       "\n Source:\n",
                       std::string(source.data(), source.data() + std::min(128ul, source.size())));
            throw std::runtime_error("JSON parse error");
        }
        obj.init(document);
    }
}
