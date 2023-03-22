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
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include "CsApp/CreditComponent.h"

namespace vsgCs
{
    class UI : public vsg::Inherit<vsg::Object, UI>
    {
        public:
        bool createUI(vsg::ref_ptr<vsg::Window> window,
                      vsg::ref_ptr<vsg::Viewer> viewer,
                      vsg::ref_ptr<vsg::Camera> camera,
                      vsg::ref_ptr<vsg::EllipsoidModel> ellipsoidModel,
                      vsg::ref_ptr<vsg::Options> options,
                      bool usesIon);
        vsg::ref_ptr<vsgImGui::RenderImGui> getImGui()
        {
            return _renderImGui;
        }
        void compile(vsg::ref_ptr<vsg::Window> window,
                     vsg::ref_ptr<vsg::Viewer> viewer);
        void setViewpoint(vsg::ref_ptr<vsg::LookAt> lookAt, float duration);
        protected:
        vsg::ref_ptr<vsgImGui::RenderImGui> createImGui(vsg::ref_ptr<vsg::Window> window,
                                                        vsg::ref_ptr<vsg::Viewer> viewer,
                                                        vsg::ref_ptr<vsg::Options> options,
                                                        bool in_usesIon);

        vsg::ref_ptr<vsgImGui::RenderImGui> _renderImGui;
        vsg::ref_ptr<CsApp::CreditComponent> _ionIconComponent;
        vsg::ref_ptr<vsg::Trackball> _trackball;
    };
}
