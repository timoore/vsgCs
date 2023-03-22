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

#include "vsgCs/runtimeSupport.h"
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include "vsgCs/ImageComponent.h"

#include <CesiumAsync/Future.h>

#include <map>
#include <string>
#include <optional>

namespace CsApp
{
    class CreditComponent : public vsg::Inherit<vsg::Object, CreditComponent>
    {
    public:
        CreditComponent(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::Options> options, bool in_usesIon);
        void compile(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::Viewer> viewer);
        bool operator()();
        bool usesIon;
    protected:
        vsg::ref_ptr<vsgCs::ImageComponent> ionLogo;
        // level of indirection because of deleted Future constructors?
        using ImageFuture = CesiumAsync::Future<vsgCs::ReadRemoteImageResult>;
        struct RemoteImage
        {
            std::shared_ptr<ImageFuture> imageResult;
            vsg::ref_ptr<vsgCs::ImageComponent> component;
        };
        std::map<std::string, RemoteImage> imageCache;
        vsg::ref_ptr<vsgCs::ImageComponent> getImage(std::string url);
    };
}
