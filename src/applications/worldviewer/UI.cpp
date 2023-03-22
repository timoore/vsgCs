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
#include "vsgCs/runtimeSupport.h"


using namespace vsgCs;

bool UI::createUI(vsg::ref_ptr<vsg::Window> window,
                  vsg::ref_ptr<vsg::Viewer> viewer,
                  vsg::ref_ptr<vsg::Camera> camera,
                  vsg::ref_ptr<vsg::EllipsoidModel> ellipsoidModel,
                  vsg::ref_ptr<vsg::Options>)
{
    createImGui(window);
    // Add the ImGui event handler first to handle events early
    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    _trackball = vsg::Trackball::create(camera, ellipsoidModel);
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    {
        _trackball->addKeyViewpoint(vsg::KeySymbol('1'), 51.50151088842245, -0.14181489107549874, 2000.0, 2.0); // Grenwish Observatory
        _trackball->addKeyViewpoint(vsg::KeySymbol('2'), 55.948642740309324, -3.199226855522667, 2000.0, 2.0);  // Edinburgh Castle
        _trackball->addKeyViewpoint(vsg::KeySymbol('3'), 48.858264952330764, 2.2945039609604665, 2000.0, 2.0);  // Eiffel Town, Paris
        _trackball->addKeyViewpoint(vsg::KeySymbol('4'), 52.5162603714634, 13.377684902745642, 2000.0, 2.0);    // Brandenburg Gate, Berlin
        _trackball->addKeyViewpoint(vsg::KeySymbol('5'), 30.047448591298807, 31.236319571791213, 10000.0, 2.0); // Cairo
        _trackball->addKeyViewpoint(vsg::KeySymbol('6'), 35.653099536061156, 139.74704060056993, 10000.0, 2.0); // Tokyo
        _trackball->addKeyViewpoint(vsg::KeySymbol('7'), 37.38701052699002, -122.08555895549424, 10000.0, 2.0); // Mountain View, California
        _trackball->addKeyViewpoint(vsg::KeySymbol('8'), 40.689618207006355, -74.04465595488215, 10000.0, 2.0); // Empire State Building
        _trackball->addKeyViewpoint(vsg::KeySymbol('9'), 25.997055873649554, -97.15543476551771, 1000.0, 2.0);  // Boca Chica, Taxas
        // osgEarthStyleMouseButtons
        _trackball->panButtonMask = vsg::BUTTON_MASK_1;
        _trackball->rotateButtonMask = vsg::BUTTON_MASK_2;
        _trackball->zoomButtonMask = vsg::BUTTON_MASK_3;
    }
    viewer->addEventHandler(_trackball);

    return true;
}

vsg::ref_ptr<vsgImGui::RenderImGui> UI::createImGui(vsg::ref_ptr<vsg::Window> window)
{
    _ionIconComponent = CsApp::CreditComponent::create();
    _renderImGui = vsgImGui::RenderImGui::create(window, _ionIconComponent);
    return _renderImGui;
}

void UI::setViewpoint(vsg::ref_ptr<vsg::LookAt> lookAt, float duration)
{
    _trackball->setViewpoint(lookAt, duration);
}
