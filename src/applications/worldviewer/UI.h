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
#include "CsApp/MapManipulator.h"

namespace vsgCs
{
    class UI : public vsg::Inherit<vsg::Object, UI>
    {
        public:
        bool createUI(const vsg::ref_ptr<vsg::Window>& window,
                      const vsg::ref_ptr<vsg::Viewer>& viewer,
                      const vsg::ref_ptr<vsg::Camera>& camera,
                      const vsg::ref_ptr<vsg::EllipsoidModel>& ellipsoidModel,
                      const vsg::ref_ptr<vsg::Options>& options,
                      const vsg::ref_ptr<WorldNode>& worldNode,
                      const vsg::ref_ptr<vsg::Group>& scene,
                      bool debugManipulator = false);
        vsg::ref_ptr<vsgImGui::RenderImGui> getImGui()
        {
            return _renderImGui;
        }
        void setViewpoint(const vsg::ref_ptr<vsg::LookAt>& lookAt, float duration);
        protected:
        vsg::ref_ptr<vsgImGui::RenderImGui> createImGui(const vsg::ref_ptr<vsg::Window>& window);

        vsg::ref_ptr<vsgImGui::RenderImGui> _renderImGui;
        vsg::ref_ptr<CsApp::CreditComponent> _ionIconComponent;
        vsg::ref_ptr<MapManipulator> _mapManipulator;
    };
}
