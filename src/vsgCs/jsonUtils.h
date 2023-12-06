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

#include <algorithm>
#include <map>
#include <string>

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
                       std::string(source.data(), source.data() + std::min(size_t(128), source.size())));
            throw std::runtime_error("JSON parse error");
        }
        obj.init(document);
    }

    class JSONObjectFactory : public vsg::Inherit<vsg::Object, JSONObjectFactory>
    {
        public:
        /**
         * Builder function takes JSON value, the factory, and a constructed object to initialze. If
         * that is null, then the builder should create the object.
         */
        using Builder = vsg::ref_ptr<vsg::Object> (*)(const rapidjson::Value& json,
                                                      JSONObjectFactory* factory,
                                                      const vsg::ref_ptr<vsg::Object>& object);
        void addBuilder(const std::string& name, Builder builder)
        {
            builders[name] = builder;
        }

        void removeBuilder(const std::string& name)
        {
            auto itr = builders.find(name);
            if (itr != builders.end())
            {
                builders.erase(name);
            }
        }
        /**
         * @brief Build an object based on the "Type" attribute found in its JSON representation.
         *
         * @param value the rapidjson Value object
         * @param typeOverride Use this type instead of the found type. (sublcassing?)
         * @param object Initialize this object instead of constructing a new one.
         * @returns A constructed object
         */
        vsg::ref_ptr<vsg::Object> build(const rapidjson::Value& value,
                                        const std::string& typeOverride = std::string(),
                                        const vsg::ref_ptr<vsg::Object>& object = {});
        /**
         * @brief Build or initialize an object from a JSON source (e.g. std::string).
         */
        template<typename TSource, typename TDest = rapidjson::Document>
        vsg::ref_ptr<vsg::Object> buildFromSource(TSource&& source)
        {
            TDest document;
            document.Parse(source.data(), source.size());
            if (document.HasParseError())
            {
                vsg::error("Error parsing json: error code ", document.GetParseError(),
                           " at byte ", document.GetErrorOffset(),
                           "\n Source:\n",
                           std::string(source.data(), source.data() + std::min(size_t(128), source.size())));
                throw std::runtime_error("JSON parse error");
            }
            return this->build(document);
        }
        /**
         * @brief get the standard factory
         */
        static vsg::ref_ptr<JSONObjectFactory> get();
        /**
         * Helper class to register a builder from a static object i.e., at load time.
         */
        struct Registrar
        {
            Registrar(const std::string& name, Builder builder)
            {
                JSONObjectFactory::get()->addBuilder(name, builder);
            }
        };
        // Leaving this public. If you want to mess with it, be my guest.
        std::map<std::string, Builder> builders;
    };

    // Throw std::runtime_error if property isn't found.
    std::string getStringOrError(const rapidjson::Value& json, const std::string& key,
                                 const char* errorMsg);
    inline std::string getStringOrError(const rapidjson::Value& json, const std::string& key,
                                 const std::string& errorMsg)
    {
        return getStringOrError(json, key, errorMsg.c_str());
    }
}
