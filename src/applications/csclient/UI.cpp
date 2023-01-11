#include "UI.h"

#include <vsg/all.h>
#include <vsgXchange/all.h>
#include <vsgImGui/imgui.h>

#include "vsgCs/Config.h"


using namespace vsgCs;

CSGuiComponent::CSGuiComponent(vsg::ref_ptr<vsg::Window> window,
                               vsg::ref_ptr<vsg::Options> options, bool in_usesIon)
    : usesIon(in_usesIon)
{
    vsg::Path filename("images/Cesium_ion_256.png");
    auto object = vsg::read(filename, options);
    if (auto data = object.cast<vsg::Data>(); data)
    {
        ionLogo = ImageComponent::create(window, data);
    }
}

void CSGuiComponent::compile(vsg::ref_ptr<vsg::Window>, vsg::ref_ptr<vsg::Viewer> viewer)
{
    // XXX Should probably use the context select function.
    viewer->compileManager->compile(ionLogo);
}

bool CSGuiComponent::operator()()
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

bool UI::createUI(vsg::ref_ptr<vsg::Window> window,
                  vsg::ref_ptr<vsg::Viewer> viewer,
                  vsg::ref_ptr<vsg::Camera> camera,
                  vsg::ref_ptr<vsg::EllipsoidModel> ellipsoidModel,
                  vsg::ref_ptr<vsg::Options> options,
                  bool usesIon)
{
    createImGui(window, viewer,  options, usesIon);
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

vsg::ref_ptr<vsgImGui::RenderImGui> UI::createImGui(vsg::ref_ptr<vsg::Window> window,
                                                   vsg::ref_ptr<vsg::Viewer>,
                                                   vsg::ref_ptr<vsg::Options> options,
                                                   bool usesIon)
{
    _csGuiComponent = CSGuiComponent::create(window, options, usesIon);
    _renderImGui = vsgImGui::RenderImGui::create(window, *_csGuiComponent);
    return _renderImGui;
}

void UI::compile(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::Viewer> viewer)
{
    _csGuiComponent->compile(window, viewer);
}

void UI::setViewpoint(vsg::ref_ptr<vsg::LookAt> lookAt, float duration)
{
    _trackball->setViewpoint(lookAt, duration);
}
