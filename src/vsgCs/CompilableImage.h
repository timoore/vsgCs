#pragma once

#include <vsg/commands/Command.h>
#include <vsg/state/ImageInfo.h>

namespace vsgCs {
    class CompilableImage : public vsg::Inherit<vsg::Command, CompilableImage>
    {
        public:
        CompilableImage(vsg::ref_ptr<vsg::ImageInfo> in_imageInfo)
            : imageInfo(in_imageInfo)
        {}
        void compile(vsg::Context& context) override;
        void record(vsg::CommandBuffer& commandBuffer) const override;
        vsg::ref_ptr<vsg::ImageInfo> imageInfo;
    };
}
