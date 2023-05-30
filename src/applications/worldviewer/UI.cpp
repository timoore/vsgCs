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

#include "UI.h"

#include <vsg/all.h>
#if defined(vsgXchange_found)
#include <vsgXchange/all.h>
#endif
#include <vsgImGui/imgui.h>

#include "vsgCs/Config.h"
#include "vsgCs/WorldNode.h"
#include "vsgCs/runtimeSupport.h"


using namespace vsgCs;

bool UI::createUI(const vsg::ref_ptr<vsg::Window>& window,
                  const vsg::ref_ptr<vsg::Viewer>& viewer,
                  const vsg::ref_ptr<vsg::Camera>& camera,
                  const vsg::ref_ptr<vsg::EllipsoidModel>&,
                  const vsg::ref_ptr<vsg::Options>&,
                  const vsg::ref_ptr<WorldNode>& worldNode)
{
    createImGui(window);
    // Add the ImGui event handler first to handle events early
    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    _mapManipulator = MapManipulator::create(worldNode, camera);
    viewer->addEventHandler(_mapManipulator);

    return true;
}

vsg::ref_ptr<vsgImGui::RenderImGui> UI::createImGui(const vsg::ref_ptr<vsg::Window>& window)
{
    _ionIconComponent = CsApp::CreditComponent::create();
    _renderImGui = vsgImGui::RenderImGui::create(window, _ionIconComponent);
    return _renderImGui;
}

void UI::setViewpoint(const vsg::ref_ptr<vsg::LookAt>& lookAt, float duration)
{
    if (_trackball)
    {
        _trackball->setViewpoint(lookAt, duration);
    }
}
