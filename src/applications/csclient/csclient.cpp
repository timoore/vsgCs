/* <editor-fold desc="MIT License">

Copyright(c) 2023 Timothy Moore

based on code that is

Copyright (c) 2018 Robert Osfield

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

#ifdef vsgXchange_FOUND
#include <vsgXchange/all.h>
#endif

#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include <imgui.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "vsgCs/TilesetNode.h"
#include "vsgCs/CsOverlay.h"
#include "vsgCs/OpThreadTaskProcessor.h"
#include "vsgCs/RuntimeEnvironment.h"
#include "UI.h"

void usage(const char* name)
{
    std::cout
        << "\nUsage: " << name << " <options> [model files]...\n\n"
        << "where options include:\n"
        << "--ion-asset asset_id\t asset id of tileset on ion server\n"
        << "--ion-overlay asset_id\t asset id of overlay for tileset\n"
        << "--tileset-url url\t URL for a tileset\n"
        << "--ion-endpoint-url url\t URL for an ion server. Defaults to Cesium's\n"
        << "--no-headlight\t\t Fix lighting at noon GMT in summer\n"
        << vsgCs::RuntimeEnvironment::usage()
        << "-f numFrames\t\t run for numFrames and exit\n"
        << "--hmh height\t\t horizon mountain height for ellipsoid perspective viewing\n"
        << "--disble-EllipsoidPerspective|--dep disable ellipsoid perspective\n"
        << "--poi lat lon\t\t coordinates of initial point of interest\n"
        << "--distance dist\t\t distance from point of interest\n"
        << "--help\t\t\t print this message\n";
}

int main(int argc, char** argv)
{
    try
    {
        // set up defaults and read command line arguments to override them
        vsg::CommandLine arguments(&argc, argv);

        if (arguments.read({"--help", "-h", "-?"}))
        {
            usage(argv[0]);
            return 0;
        }
        // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO related options to use when reading and writing files.
        auto environment = vsgCs::RuntimeEnvironment::get();
        auto window = environment->openWindow(arguments, "csclient");
#ifdef vsgXchange_FOUND
        // add vsgXchange's support for reading and writing 3rd party file formats
        environment->options->add(vsgXchange::all::create());
        arguments.read(environment->options);
#endif
        auto numFrames = arguments.value(-1, "-f");
        auto pathFilename = arguments.value(std::string(), "-p");
        auto horizonMountainHeight = arguments.value(0.0, "--hmh");
        bool useEllipsoidPerspective = !arguments.read({"--disble-EllipsoidPerspective", "--dep"});

        const double invalid_value = std::numeric_limits<double>::max();
        double poi_latitude = invalid_value;
        double poi_longitude = invalid_value;
        double poi_distance = invalid_value;
        while (arguments.read("--poi", poi_latitude, poi_longitude)) {};
        while (arguments.read("--distance", poi_distance)) {};
        if (int log_level = 0; arguments.read("--log-level", log_level))
        {
            vsg::Logger::instance()->level = static_cast<vsg::Logger::Level>(log_level);
        }
        auto ionAsset = arguments.value<int64_t>(-1L, "--ion-asset");
        auto ionOverlay = arguments.value<int64_t>(-1L, "--ion-overlay");
        auto tilesetUrl = arguments.value(std::string(), "--tileset-url");
        auto ionEndpointUrl = arguments.value(std::string(), "--ion-endpoint-url");
        bool useHeadlight = !(arguments.read("--no-headlight"));

        if (arguments.errors())
        {
            return arguments.writeErrorMessages(std::cerr);
        }

        auto vsg_scene = vsg::Group::create();

        auto ambientLight = vsg::AmbientLight::create();
        ambientLight->name = "ambient";
        ambientLight->color.set(1.0, 1.0, 1.0);
        ambientLight->intensity = 0.2;
        vsg_scene->addChild(ambientLight);
        if (!useHeadlight)
        {
            auto directionalLight = vsg::DirectionalLight::create();
            directionalLight->name = "directional";
            directionalLight->color.set(1.0, 1.0, 1.0);
            directionalLight->intensity = 1.0;
            directionalLight->direction.set( -.9397, 0.0, -.340);
            vsg_scene->addChild(directionalLight);
        }

        // read any vsg files
        for (int i = 1; i < argc; ++i)
        {
            vsg::Path filename = arguments[i];

            auto object = vsg::read(filename, environment->options);
            if (auto node = object.cast<vsg::Node>(); node)
            {
                vsg_scene->addChild(node);
            }
            else if (object)
            {
                std::cout << "Unable to view object of type " << object->className() << std::endl;
            }
            else
            {
                std::cout << "Unable to load file " << filename << std::endl;
            }
        }
        vsgCs::startup();
        vsgCs::TilesetSource source;
        if (!tilesetUrl.empty())
        {
            source.url = tilesetUrl;
        }
        else
        {
            if (ionAsset < 0)
            {
                std::cout << "No valid Ion asset\n";
                return 1;
            }
            source.ionAssetID = ionAsset;
            source.ionAccessToken = environment->ionAccessToken;
            if (!ionEndpointUrl.empty())
            {
                source.ionAssetEndpointUrl = ionEndpointUrl;
            }
        }
        // create the viewer and assign window(s) to it
        auto viewer = vsg::Viewer::create();
        if (!window)
        {
            std::cout << "Could not create windows." << std::endl;
            return 1;
        }
        Cesium3DTilesSelection::TilesetOptions tileOptions;
        tileOptions.enableOcclusionCulling = false;
        tileOptions.forbidHoles = true;
        auto tilesetNode = vsgCs::TilesetNode::create(environment->features, source, tileOptions, environment->options);
        auto ellipsoidModel = vsg::EllipsoidModel::create();
        tilesetNode->setObject("EllipsoidModel", ellipsoidModel);
        // XXX need to detach this from the tileset before the program exits
        vsg::ref_ptr<vsgCs::CsIonRasterOverlay> csoverlay;
        if (ionOverlay > 0)
        {
            csoverlay = vsgCs::CsIonRasterOverlay::create(ionOverlay, environment->ionAccessToken);
            csoverlay->addToTileset(tilesetNode);
        }

        vsg_scene->addChild(tilesetNode);
        viewer->addWindow(window);
        environment->setViewer(viewer);

        // compute the bounds of the scene graph to help position camera
        // XXX not yet
#if 0
        vsg::ComputeBounds computeBounds;
        vsg_scene->accept(computeBounds);
        vsg::dvec3 centre = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
        double radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;
#endif
        vsg::dvec3 centre(0.0, 0.0, 0.0);
        double radius = ellipsoidModel->radiusEquator();

        double nearFarRatio = 0.0005;

        // set up the camera
        vsg::ref_ptr<vsg::LookAt> lookAt;
        vsg::ref_ptr<vsg::ProjectionMatrix> perspective;
        bool setViewpointAfterLoad = false;
        if (poi_latitude != invalid_value && poi_longitude != invalid_value)
        {
            double height = (poi_distance != invalid_value) ? poi_distance : radius * 3.5;
            auto ecef = ellipsoidModel->convertLatLongAltitudeToECEF({poi_latitude, poi_longitude, 0.0});
            auto ecef_normal = vsg::normalize(ecef);

            centre = ecef;
            vsg::dvec3 eye = centre + ecef_normal * height;
            vsg::dvec3 up = vsg::normalize(vsg::cross(ecef_normal, vsg::cross(vsg::dvec3(0.0, 0.0, 1.0), ecef_normal)));

            // set up the camera
            lookAt = vsg::LookAt::create(eye, centre, up);
        }
        else
        {
            lookAt = vsg::LookAt::create(centre + vsg::dvec3(radius * 3.5, 0.0, 0.0), centre, vsg::dvec3(0.0, 0.0, 1.0));
            setViewpointAfterLoad = true;
        }

        if (useEllipsoidPerspective)
        {
            perspective = vsg::EllipsoidPerspective::create(lookAt, ellipsoidModel, 30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio, horizonMountainHeight);
        }
        else
        {
            perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio * radius, radius * 4.5);
        }
        auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));
        auto ui = vsgCs::UI::create();
        ui->createUI(window, viewer, camera, ellipsoidModel, environment->options);
        auto commandGraph = vsg::CommandGraph::create(window);
        auto renderGraph = vsg::RenderGraph::create(window);
        renderGraph->setClearValues({{0.02899f, 0.02899f, 0.13321f}});
        commandGraph->addChild(renderGraph);

        auto view = vsg::View::create(camera);
        if (useHeadlight)
        {
            view->addChild(vsg::createHeadlight());
        }
        view->addChild(vsg_scene);
        renderGraph->addChild(view);

        renderGraph->addChild(ui->getImGui());
        viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});
        viewer->compile();

        tilesetNode->initialize(viewer);
        
        // rendering main loop
        while (viewer->advanceToNextFrame() && (numFrames < 0 || (numFrames--) > 0))
        {
            if (setViewpointAfterLoad
                && tilesetNode->getTileset()
                && tilesetNode->getTileset()->getRootTile())
            {
                lookAt = vsgCs::makeLookAtFromTile(tilesetNode->getTileset()->getRootTile(),
                                                   poi_distance);
                ui->setViewpoint(lookAt, 1.0);
                setViewpointAfterLoad = false;
            }
            // pass any events into EventHandlers assigned to the Viewer
            viewer->handleEvents();

            environment->update();
            viewer->update();

            viewer->recordAndSubmit();

            viewer->present();
        }
        tilesetNode->shutdown();
        vsgCs::shutdown();
    }
    catch (const vsg::Exception& ve)
    {
        for (int i = 0; i < argc; ++i)
        {
            std::cerr << argv[i] << " ";
        }
        std::cerr << "\n[Exception] - " << ve.message << " result = " << ve.result << std::endl;
        return 1;
    }

    // clean up done automatically thanks to ref_ptr<>
    // Hah!
    return 0;
}
