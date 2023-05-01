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

// A very preliminary implementation of 3D Tiles styling. No JavaScript
// expressions yet.

#include "ModelBuilder.h"

#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>

#include <optional>
namespace vsgCs
{

    class Stylist;

    /**
     * @brief Encapsulate styling declarations and prepare any data needed for a Stylist class to
     * modify model data.
     */
    class Styling : public vsg::Inherit<vsg::Object, Styling>
    {
    public:
        Styling();
        Styling(bool in_show);
        Styling(bool in_show, const vsg::vec4& in_color);
        virtual vsg::ref_ptr<Stylist> getStylist(ModelBuilder* in_modelBuilder);
        bool show;
        std::optional<vsg::vec4> color;
    };

    /**
     * @brief A Stylist is created for each model (tile), mostly to give it the ability to preprocess
     * the tile's feature tables.
     */
    class Stylist : public vsg::Inherit<vsg::Object, Stylist>
    {
        public:
        Stylist(Styling* in_styling,
                ModelBuilder* in_modelBuilder);
        vsg::ref_ptr<Styling> styling;
        struct PrimitiveStyling
        {
            bool show = true;
            vsg::ref_ptr<vsg::Data> colors;
            VkVertexInputRate vertexRate = VK_VERTEX_INPUT_RATE_VERTEX;
        };
        PrimitiveStyling getStyling(const CesiumGltf::MeshPrimitive* primitive);

        ModelBuilder* modelBuilder;
        std::vector<std::optional<vsg::vec4>> featureColors;
    };
}
