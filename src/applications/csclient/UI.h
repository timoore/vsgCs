#pragma once

#include <vsg/all.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include "vsgCs/ImageComponent.h"

namespace vsgCs
{
    class CSGuiComponent : public vsg::Inherit<vsg::Object, CSGuiComponent>
    {
    public:
        CSGuiComponent(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::Options> options, bool in_usesIon);
        void compile(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::Viewer> viewer);
        bool operator()();
        bool usesIon;
    protected:
        vsg::ref_ptr<ImageComponent> ionLogo;
    };
    
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
        vsg::ref_ptr<CSGuiComponent> _csGuiComponent;
        vsg::ref_ptr<vsg::Trackball> _trackball;
    };
}
