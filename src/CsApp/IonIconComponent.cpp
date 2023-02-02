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
#include "vsgCs/RuntimeEnvironment.h"

#include <tinyxml2.h>
#include <algorithm>


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

// This could quickly get out of hand... Fix the breakage we know about i.e., img with no closing />
namespace
{
    void cleanHtml(std::string& input)
    {
        const std::string img("<img");
        auto searchStartItr = input.begin();
        decltype(searchStartItr) imgItr;
        while ((imgItr = std::search(searchStartItr, input.end(),
                                  img.begin(), img.end(),
                                  [](unsigned char c1, unsigned char c2)
                                  {
                                      return std::toupper(c1) == std::toupper(c2);
                                  })) != input.end())
        {
            auto closeItr = std::find(imgItr, input.end(),'>');
            if (closeItr == input.end())
            {
                vsg::error("no closing > for img tag: ", input);
            }
            auto ltPos = closeItr - input.begin();
            if ( *(closeItr - 1) != '/')
            {
                input.insert(closeItr, '/');
                searchStartItr = input.begin() + ltPos + 1;
            }
            else
            {
                searchStartItr = closeItr;
            }
        }
        return;
    }
}

// Implements a cache for remote images. If we don't have the image already,
// make a future for it. When the future is resolved, then we can keep the image
// in the map and return it.

vsg::ref_ptr<vsgCs::ImageComponent> IonIconComponent::getImage(std::string url)
{
    auto env = vsgCs::RuntimeEnvironment::get();
    auto insertResult = imageCache.insert(std::pair(url, RemoteImage()));
    RemoteImage& remoteImage = insertResult.first->second;
    if (insertResult.second)
    {
        // New entry - image will be available later
        remoteImage.imageResult = std::make_shared<ImageFuture>(vsgCs::readRemoteImage(url));
    }

    if (remoteImage.component)
    {
        return remoteImage.component;
    }
    else if (remoteImage.imageResult)
    {
        vsg::ref_ptr<vsgCs::ImageComponent> retval;
        if (remoteImage.imageResult->isReady())
        {
            auto remoteResult = remoteImage.imageResult->wait();
            if (!remoteResult.info)
            {
                for (auto& err : remoteResult.errors)
                {
                    vsg::error(err);
                }
            }
            else
            {
                remoteImage.component = vsgCs::ImageComponent::create(ionLogo->_window, remoteResult.info);
                env->getViewer()->compileManager->compile(remoteImage.component);
                retval = remoteImage.component;
            }
            remoteImage.imageResult.reset();
        }
        return retval;
    }
    else
    {
        // The Future was resolved, but with an error.
        return {};
    }
}

// This is the lamest renderer ever: render a small line of HTML using ImGui.

bool IonIconComponent::operator()()
{
    auto env = vsgCs::RuntimeEnvironment::get();
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

    auto creditSystem = vsgCs::RuntimeEnvironment::get()->getTilesetExternals()->pCreditSystem;
    auto credits = creditSystem->getCreditsToShowThisFrame();
    for (auto& credit : credits)
    {
        if (creditSystem->shouldBeShownOnScreen(credit))
        {
            auto html = creditSystem->getHtml(credit);
            if (html.empty())
            {
                continue;
            }
            visibleComponents = true;
            cleanHtml(html);
            tinyxml2::XMLDocument doc;
            tinyxml2::XMLError xerr = doc.Parse(html.c_str());
            if (xerr != 0)
            {
                vsg::error("tinyxml2 error ", std::to_string(int32_t(xerr)), " ", html);
                break;
            }
            auto node = doc.FirstChildElement();
            if (std::strcmp(node->Name(), "span") == 0)
            {
                node = node->FirstChildElement();
            }
            if (std::strcmp(node->Name(), "a") == 0)
            {
                std::string href(node->Attribute("href"));
                auto aContents = node->FirstChild();
                if (auto asText = aContents->ToText(); asText)
                {
                    ImGui::Text("%s", asText->Value());
                    ImGui::SameLine();

                }
                else
                {
                    auto element = aContents->ToElement();
                    if (element && strcmp(element->Name(), "img") == 0)
                    {
                        auto src = element->Attribute("src");
                        auto alt = element->Attribute("alt");
                        auto component = getImage(src);
                        if (component)
                        {
                            auto height = component->height;
                            (*component)();
                            ImGui::SameLine();
                            auto pos = ImGui::GetCursorPos();
                            ImGui::SetCursorPosY(pos.y + height / 2.0 - ImGui::GetFontSize() / 2);
                        }
                        else if (alt)
                        {
                            ImGui::Text("%s", alt);
                            ImGui::SameLine();

                        }
                    }
                }
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    return visibleComponents;
}
