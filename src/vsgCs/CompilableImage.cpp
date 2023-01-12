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
