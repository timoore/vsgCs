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
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/utils/Builder.h>

#if defined(vsgXchange_found)
#include <vsgXchange/all.h>
#endif
#include <imgui.h>

#include "vsgCs/Config.h"
#include "vsgCs/WorldNode.h"
#include "vsgCs/runtimeSupport.h"


namespace vsgCs
{

// For debugging manipulator

    vsg::ref_ptr<vsg::MatrixTransform> createDot()
    {
        auto builder = vsg::Builder::create();
        vsg::StateInfo stateInfo;
        stateInfo.lighting = false;
        vsg::GeometryInfo geomInfo;
        geomInfo.dx.set(0.005f, 0.0f, 0.0f);
        geomInfo.dy.set(0.0f, 0.005f, 0.0f);
        geomInfo.dz.set(0.0f, 0.0f, 0.005f);
        geomInfo.color = vsg::vec4(1.0, 0.0, 1.0, 1.0);

        auto sphere = builder->createSphere(geomInfo, stateInfo);
        auto transform = vsg::MatrixTransform::create();
        transform->addChild(sphere);
        return transform;
    }

    class UpdateCenterOperation : public vsg::Inherit<vsg::Operation, UpdateCenterOperation>
    {
    public:
        void run() override
        {

            auto camera = manipulator->_camera;
            auto lookAt = ref_ptr_cast<vsg::LookAt>(camera->viewMatrix);
            if (!lookAt)
            {
                return;
            }

            auto pos = manipulator->_state.center;
            auto scaleFactor = vsg::length(pos - lookAt->eye);
            dot->matrix = vsg::translate(pos) * vsg::scale(scaleFactor);
        }
        vsg::ref_ptr<MapManipulator> manipulator;
        vsg::ref_ptr<vsg::MatrixTransform> dot;
    };

    bool UI::createUI(const vsg::ref_ptr<vsg::Window>& window,
                      const vsg::ref_ptr<vsg::Viewer>& viewer,
                      const vsg::ref_ptr<vsg::Camera>& camera,
                      const vsg::ref_ptr<vsg::EllipsoidModel>& ellipsoidModel,
                      const vsg::ref_ptr<vsg::Options>&,
                      const vsg::ref_ptr<WorldNode>& worldNode,
                      const vsg::ref_ptr<vsg::Group>& scene,
                      bool debugManipulator)
    {
        createImGui(window);
        // Add the ImGui event handler first to handle events early
        viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
        viewer->addEventHandler(vsg::CloseHandler::create(viewer));
        if (ellipsoidModel)
        {
            _mapManipulator = MapManipulator::create(worldNode, camera);
            viewer->addEventHandler(_mapManipulator);
            if (debugManipulator)
            {
                auto manipCenter = createDot();
                auto updateCenter = UpdateCenterOperation::create();
                updateCenter->manipulator = _mapManipulator;
                updateCenter->dot = manipCenter;
                scene->addChild(manipCenter);
                viewer->addUpdateOperation(updateCenter, vsg::UpdateOperations::ALL_FRAMES);
            }
        }
        else
        {
            _trackball = vsg::Trackball::create(camera);
            viewer->addEventHandler(_trackball);
        }

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
        if (_mapManipulator)
        {
            _mapManipulator->setHome(lookAt->center, length( lookAt->center - lookAt->eye));
            _mapManipulator->home();
        }
        else if (_trackball)
        {
            _trackball->setViewpoint(lookAt, duration);
        }
    }
}
