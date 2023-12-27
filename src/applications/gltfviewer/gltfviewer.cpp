#include "vsgCs/GltfLoader.h"
#include "vsgCs/pbr.h"
#include "vsgCs/RuntimeEnvironment.h"
#include "vsgCs/runtimeSupport.h"

#include <vsg/all.h>

#include <iostream>

int main(int argc, char** argv)
{
    vsg::CommandLine arguments(&argc, argv);
    auto environment = vsgCs::RuntimeEnvironment::get();
    auto window = environment->openWindow(arguments, "worldviewer");


    bool add_amient = true;
    bool add_directional = true;
    bool add_point = true;
    bool add_spotlight = true;
    bool add_headlight = arguments.read("--headlight");
    if (add_headlight || arguments.read({"--no-lights", "-n"}))
    {
        add_amient = false;
        add_directional = false;
        add_point = false;
        add_spotlight = false;
    }

    // Set up the view shading parameters in the vsgCs way
    auto scene = vsg::StateGroup::create();
        // Bind the lighting for the whole world
    auto bindViewDescriptorSets
        = vsg::BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                              vsgCs::RuntimeEnvironment::get()->genv->overlayPipelineLayout,
                                              vsgCs::pbr::VIEW_DESCRIPTOR_SET);
    scene->add(bindViewDescriptorSets);
    auto loader = vsgCs::GltfLoader::create();
    
    if (argc>1)
    {
        vsg::Path filename = argv[1];
        auto model = vsgCs::ref_ptr_cast<vsg::Node>(loader->read(filename, vsgCs::RuntimeEnvironment::get()->options));
        if (!model)
        {
            std::cout<<"Faled to load "<<filename<<std::endl;
            return 1;
        }
        scene->addChild(model);
    }
    
    // compute the bounds of the scene graph to help position camera
    auto bounds = vsg::visit<vsg::ComputeBounds>(scene).bounds;

    if (add_amient || add_directional || add_point || add_spotlight || add_headlight)
    {
        auto span = vsg::length(bounds.max - bounds.min);
        auto group = vsg::Group::create();
        group->addChild(scene);

        // ambient light
        if (add_amient)
        {
            auto ambientLight = vsg::AmbientLight::create();
            ambientLight->name = "ambient";
            ambientLight->color.set(1.0, 1.0, 1.0);
            ambientLight->intensity = 0.01;
            group->addChild(ambientLight);
        }

        // directional light
        if (add_directional)
        {
            auto directionalLight = vsg::DirectionalLight::create();
            directionalLight->name = "directional";
            directionalLight->color.set(1.0, 1.0, 1.0);
            directionalLight->intensity = 0.15;
            directionalLight->direction.set(0.0, -1.0, -1.0);
            group->addChild(directionalLight);
        }

        // point light
        if (add_point)
        {
            auto pointLight = vsg::PointLight::create();
            pointLight->name = "point";
            pointLight->color.set(1.0, 1.0, 0.0);
            pointLight->intensity = span*0.5;
            pointLight->position.set(bounds.min.x, bounds.min.y, bounds.max.z + span*0.3);

            // enable culling of the point light by decorating with a CullGroup
            auto cullGroup = vsg::CullGroup::create();
            cullGroup->bound.center = pointLight->position;
            cullGroup->bound.radius = span;

            cullGroup->addChild(pointLight);

            group->addChild(cullGroup);
        }

        // spot light
        if (add_spotlight)
        {
            auto spotLight = vsg::SpotLight::create();
            spotLight->name = "spot";
            spotLight->color.set(0.0, 1.0, 1.0);
            spotLight->intensity = span*0.5;
            spotLight->position.set(bounds.max.x + span*0.1, bounds.min.y - span*0.1, bounds.max.z + span*0.3);
            spotLight->direction = (bounds.min+bounds.max)*0.5 - spotLight->position;
            spotLight->innerAngle = vsg::radians(8.0);
            spotLight->outerAngle = vsg::radians(9.0);

            // enable culling of the spot light by decorating with a CullGroup
            auto cullGroup = vsg::CullGroup::create();
            cullGroup->bound.center = spotLight->position;
            cullGroup->bound.radius = span;

            cullGroup->addChild(spotLight);

            group->addChild(cullGroup);
        }

        if (add_headlight)
        {
            auto ambientLight = vsg::AmbientLight::create();
            ambientLight->name = "ambient";
            ambientLight->color.set(1.0, 1.0, 1.0);
            ambientLight->intensity = 0.1;

            auto directionalLight = vsg::DirectionalLight::create();
            directionalLight->name = "head light";
            directionalLight->color.set(1.0, 1.0, 1.0);
            directionalLight->intensity = 0.9;
            directionalLight->direction.set(0.0, 0.0, -1.0);

            auto absoluteTransform = vsg::AbsoluteTransform::create();
            absoluteTransform->addChild(ambientLight);
            absoluteTransform->addChild(directionalLight);

            group->addChild(absoluteTransform);
        }
        scene->addChild(group);
    }


    // create the viewer and assign window(s) to it
    auto viewer = vsg::Viewer::create();

    if (!window)
    {
        std::cout << "Could not create windows." << std::endl;
        return 1;
    }

    viewer->addWindow(window);

    vsg::ref_ptr<vsg::LookAt> lookAt;

    vsg::dvec3 centre = (bounds.min + bounds.max) * 0.5;
    double radius = vsg::length(bounds.max - bounds.min) * 0.6;

    // set up the camera
    lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, -radius * 3.5, 0.0), centre, vsg::dvec3(0.0, 0.0, 1.0));

    double nearFarRatio = 0.001;
    auto perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio * radius, radius * 10.0);

    auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));

    // add the camera and scene graph to View
    auto view = vsg::View::create();
    view->camera = camera;
    view->addChild(scene);

    // add close handler to respond the close window button and pressing escape
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->addEventHandler(vsg::Trackball::create(camera));

    auto renderGraph = vsg::RenderGraph::create(window, view);
    // The classic VSG background, translated into sRGB values.
    renderGraph->setClearValues({{0.02899f, 0.02899f, 0.13321f}});

    auto commandGraph = vsg::CommandGraph::create(window, renderGraph);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

    viewer->compile();

    auto startTime = vsg::clock::now();
    double numFramesCompleted = 0.0;

    // rendering main loop
    while (viewer->advanceToNextFrame())
    {
        // pass any events into EventHandlers assigned to the Viewer
        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();

        numFramesCompleted += 1.0;
    }

    auto duration = std::chrono::duration<double, std::chrono::seconds::period>(vsg::clock::now() - startTime).count();
    if (numFramesCompleted > 0.0)
    {
        std::cout << "Average frame rate = " << (numFramesCompleted / duration) << std::endl;
    }

    return 0;
}
