#include "CompilableImage.h"
#include <vsg/all.h>

using namespace vsgCs;

// Copied from DescriptorImage
void CompilableImage::compile(vsg::Context &context)
{

    imageInfo->computeNumMipMapLevels();

    if (imageInfo->sampler) imageInfo->sampler->compile(context);
    if (imageInfo->imageView)
    {
        auto& imageView = *imageInfo->imageView;
        imageView.compile(context);

        if (imageView.image && imageView.image->syncModifiedCount(context.deviceID))
        {
            auto& image = *imageView.image;
            context.copy(image.data, imageInfo, image.mipLevels);
        }
    }
}

void CompilableImage::record(vsg::CommandBuffer&) const
{
    vsg::fatal("CompilableImage is not a real command and can't be recorded.");
}
