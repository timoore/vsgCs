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

#include <vsg/all.h>
#include "Export.h"

// A component that can render an image in Dear ImGui.

namespace vsgCs
{
    // This is a subclass of command so that it can be compiled.
    class ImageComponent : public vsg::Inherit<vsg::Command, ImageComponent>
    {
    public:
        ImageComponent(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::Data> texData);
        ImageComponent(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::ImageInfo> info);
        bool operator()();
        void compile(vsg::Context& context) override;
        void record(vsg::CommandBuffer& commandBuffer) const override;
        vsg::ref_ptr<vsg::DescriptorSet> descriptorSet;
        uint32_t height;
        uint32_t width;
        vsg::ref_ptr<vsg::Window> _window;
    };
}
