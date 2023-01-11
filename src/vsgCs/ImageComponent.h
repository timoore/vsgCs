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
        bool operator()();
        void compile(vsg::Context& context) override;
        void record(vsg::CommandBuffer& commandBuffer) const override;
        vsg::ref_ptr<vsg::DescriptorSet> descriptorSet;
        uint32_t height;
        uint32_t width;
    protected:
        vsg::ref_ptr<vsg::Window> _window;
    };
}
