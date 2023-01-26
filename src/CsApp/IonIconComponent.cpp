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

#include <vsg/all.h>
#include <vsgImGui/imgui.h>

#include "IonIconComponent.h"
#include "vsgCs/Config.h"
#include "vsgCs/runtimeSupport.h"

using namespace CsApp;

IonIconComponent::IonIconComponent(vsg::ref_ptr<vsg::Window> window,
                               vsg::ref_ptr<vsg::Options> options, bool in_usesIon)
    : usesIon(in_usesIon)
{
    vsg::Path filename("images/Cesium_ion_256.png");
    auto data = vsgCs::readImageFile(filename, options);
    if (data)
    {
        ionLogo = vsgCs::ImageComponent::create(window, data);
    }
}

void IonIconComponent::compile(vsg::ref_ptr<vsg::Window>, vsg::ref_ptr<vsg::Viewer> viewer)
{
    // XXX Should probably use the context select function.
    viewer->compileManager->compile(ionLogo);
}

bool IonIconComponent::operator()()
{
    bool visibleComponents = false;
    // Shamelessly copied from imgui_demo.cpp simple overlay
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_pos.x + PAD;
    window_pos.y = work_pos.y + work_size.y - PAD;
    window_pos_pivot.x =  0.0f;
    window_pos_pivot.y =  1.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("vsgCS UI", nullptr, window_flags);
    if (usesIon)
    {
        (*ionLogo)();
        visibleComponents = true;
    }
    ImGui::End();
    ImGui::PopStyleVar();
    return visibleComponents;
}
