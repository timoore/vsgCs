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
#include <vsgImGui/imgui.h>

#include <gsl/util>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

#include "vsgCs/TilesetNode.h"
#include "vsgCs/CsOverlay.h"
#include "vsgCs/GeoNode.h"
#include "vsgCs/jsonUtils.h"
#include "vsgCs/OpThreadTaskProcessor.h"
#include "vsgCs/Tracing.h"
#include "vsgCs/RuntimeEnvironment.h"
#include "vsgCs/WorldNode.h"
#include "UI.h"
#include "CsApp/CsViewer.h"

void usage(const char* name)
{
    std::cout
        << "\nUsage: " << name << " <options> [model files]...\n\n"
        << "where options include:\n"
        << "--world-file file\t\t JSON world file"
        << "--headlight\t\t Fix lighting in viewing direction\n"
        << vsgCs::RuntimeEnvironment::usage()
        << "-f numFrames\t\t run for numFrames and exit\n"
        << "--hmh height\t\t horizon mountain height for ellipsoid perspective viewing\n"
        << "--disble-EllipsoidPerspective|--dep disable ellipsoid perspective\n"
        << "--poi lat lon\t\t coordinates of initial point of interest\n"
        << "--distance dist\t\t distance from point of interest\n"
        << "--time HH::MM\t\t time in UTC (default 12:00)\n"
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
        // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO related options
        // to use when reading and writing files.
        // vsgCs::RuntimeEnvironment manages parsing of common arguments, initialization of the
        // Vulkan environment, and the Cesium ion token (if any).
        auto environment = vsgCs::RuntimeEnvironment::get();
        auto window = environment->openWindow(arguments, "worldviewer");
#ifdef vsgXchange_FOUND
        // add vsgXchange's support for reading and writing 3rd party file formats. If vsgXchange
        // isn't found, then we will only be able to read .vsgt models.
        environment->options->add(vsgXchange::all::create());
        arguments.read(environment->options);
#endif
        // The world file specifies the tilesets to display.
        auto worldFile = arguments.value(std::string(), "--world-file");
        // numFrames and pathFilename come from vsgViewer. They provide a way to display a
        // "flythrough." At the moment they are not useful because a scene will usually take some
        // time to load.
        auto numFrames = arguments.value(-1, "-f");
        auto pathFilename = arguments.value(std::string(), "-p");
        // Options for the VSG trackball manipulator
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
        auto timeString = arguments.value(std::string(), "--time");
        // Default is noon GMT
        std::tm tm{};
        tm.tm_year = 2023 - 1900;
        tm.tm_mon = 5;          // June
        tm.tm_mday = 21;
        tm.tm_hour = 12;
        if (!timeString.empty())
        {
            std::stringstream ss(timeString);
            ss >> std::get_time(&tm, "%R");
            if (ss.fail())
            {
                vsg::fatal("Invalid time string ", timeString);
            }
        }
        bool useHeadlight = arguments.read({"--headlight"});
        auto shadowMaps = arguments.value<uint32_t>(0, "--shadow-maps");
        auto maxShadowDistance = arguments.value<double>(10000.0, "--sd");
        bool debugManipulator = arguments.read({"--debug-manipulator"});

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
            // Noon GMT on a summer day
            auto directionalLight = vsg::DirectionalLight::create();
            directionalLight->name = "directional";
            directionalLight->color.set(1.0, 1.0, 1.0);
            directionalLight->intensity = 1.0;
            // high Summer
            float hourAngle = (tm.tm_hour + tm.tm_min / 60.0) / 24.0 * 2.0 * vsg::PIf - vsg::PIf;
            vsg::quat hourRot;
            hourRot.set(-hourAngle, vsg::vec3(0.0f, 0.0f, 1.0f));
            vsg::vec3 noon( -.9397, 0.0, -.340);
            vsg::vec3 atTime = hourRot * noon;
            directionalLight->direction.set(atTime.x, atTime.y, atTime.z);
            directionalLight->shadowMaps = shadowMaps;
            vsg_scene->addChild(directionalLight);
        }
        vsg::ref_ptr<vsg::StateGroup> modelRoot;
        if (argc > 1)
        {
            modelRoot = createModelRoot(environment);
        }
        // Read GeoNode json files
        for (int i = 1; i < argc; ++i)
        {
            vsg::Path filename = arguments[i];
            auto jsonSource = vsgCs::readFile(filename, environment->options);
            auto object = vsgCs::JSONObjectFactory::get()->buildFromSource(jsonSource);
            if (auto node = vsgCs::ref_ptr_cast<vsg::Node>(object))
            {
                modelRoot->addChild(node);
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
        if (worldFile.empty())
        {
            vsg::fatal("no world file");
        }
        auto worldJson = vsgCs::readFile(worldFile);
        // Do early cesium-native initialization
        vsgCs::startup();
        // create the VSG viewer and assign window(s) to it
        auto viewer = CsApp::CsViewer::create();

        if (!window)
        {
            std::cout << "Could not create windows." << std::endl;
            return 1;
        }
        // Create the World with its tilesets
        auto worldNode
            = vsgCs::ref_ptr_cast<vsgCs::WorldNode>(vsgCs::JSONObjectFactory::get()
                                                    ->buildFromSource(worldJson));
        // VSG's EllipsoidModel provides ellipsoid parameters (e.g. WGS84), mostly for the benefit
        // of the trackball manipulator.
        auto ellipsoidModel = vsg::EllipsoidModel::create();
        worldNode->setObject("EllipsoidModel", ellipsoidModel);
        vsg_scene->addChild(worldNode);
        if (modelRoot)
        {
            vsg_scene->addChild(modelRoot);
        }
        viewer->addWindow(window);
        // vsgCS::RuntimeEnvironment needs the vsg::Viewer object for creation of Vulkan objects
        environment->setViewer(viewer);
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
            // Establish a viewpoint when enough of the world has been loaded to know its bounds.
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
        // Create this application's user interface, including the trackball manipulator and the
        // graphical overlay.
        auto ui = vsgCs::UI::create();
        ui->createUI(window, viewer, camera, ellipsoidModel, environment->options, worldNode, vsg_scene,
                     debugManipulator);
        // Basic VSG objects for rendering
        auto commandGraph = vsg::CommandGraph::create(window);
        auto renderGraph = vsg::RenderGraph::create(window);
        // The classic VSG background, translated into sRGB values.
        renderGraph->setClearValues({{0.02899f, 0.02899f, 0.13321f}});
        commandGraph->addChild(renderGraph);

        auto view = vsg::View::create(camera);
        view->viewDependentState->maxShadowDistance = maxShadowDistance;
        if (useHeadlight)
        {
            view->addChild(vsg::createHeadlight());
        }
        view->addChild(vsg_scene);
        renderGraph->addChild(view);
        // Attach the ImGui graphical interface
        renderGraph->addChild(ui->getImGui());
        viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

        // Perform any late initialization of TilesetNode objects. Most importantly, this tracks VSG
        // cameras so that they can be used by cesium-native to determine visible tiles.
        worldNode->initialize(viewer);

        // Compile everything we can at this point.
        //
        // best practice is to tell the viewer what resources to allocate in the viewer.compile() call via ResourceHints
        //
        // auto resourceHints = vsg::ResourceHints::create();
        // resourceHints->numShadowMapsRange = {shadowMaps, 64};
        // resourceHints->maxSlot = 4;
        // viewer->compile(resourceHints);
        viewer->compile();

        auto lastAct = gsl::finally([worldNode]() {
            vsgCs::shutdown();
            worldNode->shutdown();});

        // rendering main loop
        while (viewer->advanceToNextFrame() && (numFrames < 0 || (numFrames--) > 0))
        {
            if (setViewpointAfterLoad
                && worldNode->getRootTile())
            {
                lookAt = vsgCs::makeLookAtFromTile(worldNode->getRootTile(),
                                                   poi_distance);
                ui->setViewpoint(lookAt, 1.0);
                setViewpointAfterLoad = false;
            }
            // pass any events into EventHandlers assigned to the Viewer
            viewer->handleEvents();
            // XXX This should be moved to vsg::Viewer update operation.
            environment->update();
            {
                VSGCS_ZONESCOPEDN("viewer update");
                viewer->update();
            }
            {
                VSGCS_ZONESCOPEDN("viewer record");
                viewer->recordAndSubmit();
            }

            viewer->present();
            VSGCS_FRAMEMARK;
        }
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
