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


#include "CsApp/GeneralPerspective.h"
#include <vsg/all.h>
#include <vsg/state/ViewportState.h>
#include <vulkan/vulkan_core.h>

#ifdef vsgXchange_FOUND
#include <vsgXchange/all.h>
#endif

#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include <imgui.h>

#include <gsl/util>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include "vsgCs/CppAllocator.h"
#endif

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>

#include "vsgCs/GeoNode.h"
#include "vsgCs/GltfLoader.h"
#include "vsgCs/jsonUtils.h"
#include "vsgCs/TilesetNode.h"
#include "vsgCs/Tracing.h"
#include "vsgCs/TracingCommandGraph.h"
#include "vsgCs/RuntimeEnvironment.h"
#include "vsgCs/WorldNode.h"
#include "UI.h"
#include "CsApp/CsViewer.h"

namespace
{
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

class VsgCsScenegraphBuilder
{
public:
    VsgCsScenegraphBuilder(vsg::CommandLine& arguments_,
                           const vsg::ref_ptr<vsgCs::RuntimeEnvironment>& env_)
        : arguments(arguments_), env(env_)
    {
        createScenegraph();
    }
    vsg::ref_ptr<vsgCs::WorldNode> worldNode;
    vsg::ref_ptr<vsg::StateGroup> xchangeModels;
    std::vector<vsg::ref_ptr<vsgCs::TilesetNode>> tilesetNodes;

    void createScenegraph()
    {
        for (int i = 1; i < arguments.argc(); ++i)
        {
            std::string argString(arguments[i]);
            if (argString == "--world-file")
            {
                continue;
            }
            if (argString.ends_with("tileset.json"))
            {
                if (auto tilesetNode  = createTileset(argString))
                {
                    tilesetNodes.push_back(tilesetNode);
                }
            }
            else if (argString.ends_with(".json"))
            {
                addVsgCsObject(arguments[i]);
            }
            else
            {
                vsg::Path filename = arguments[i];
                if (auto node = vsgCs::ref_ptr_cast<vsg::Node>(vsg::read(filename, env->options)))
                {
                    addToXchangeModels(node);
                }
            }
        }
        if (!worldNode && !tilesetNodes.empty())
        {
            worldNode = vsgCs::WorldNode::create();
        }
        worldNode->tilesetNodes().insert(worldNode->tilesetNodes().end(),
                                         tilesetNodes.begin(), tilesetNodes.end());
    }

private:
    void addToXchangeModels(const vsg::ref_ptr<vsg::Node>& node)
    {
        if (!xchangeModels)
        {
            xchangeModels = createModelRoot(env);
        }
        xchangeModels->addChild(node);
    }

    void addVsgCsObject(const std::string& fileName)
    {
        auto jsonSource = vsgCs::readFile(fileName, env->options);
        auto object = vsgCs::JSONObjectFactory::get()->buildFromSource(jsonSource);
        if (!object)
        {
            std::cout << "Unable to load file " << fileName << '\n';
        }
        if (auto maybeWorldNode = vsgCs::ref_ptr_cast<vsgCs::WorldNode>(object))
        {
            worldNode = maybeWorldNode;
        }
        else if (auto maybeTilesetNode = vsgCs::ref_ptr_cast<vsgCs::TilesetNode>(object))
        {
            tilesetNodes.push_back(maybeTilesetNode);
        }
        else if (auto modelNode = vsgCs::ref_ptr_cast<vsg::Node>(object))
        {
            addToXchangeModels(modelNode);
        }
        else
        {
            std::cout << "Unrecognized command line argument " << fileName << '\n';
        }
    }

    vsg::ref_ptr<vsgCs::TilesetNode> createTileset(const std::string& argString)
    {
        std::string url;
        if (argString.starts_with("https:") || argString.starts_with("http:")
            || argString.starts_with("file:"))
        {
            url = argString;
        }
        else
        {
            // It's a file, so construct an absolute file url.
            auto realPath = vsg::findFile(argString, env->options);
            if (realPath.empty())
            {
                vsg::fatal("Can't find file ", argString);
            }
            std::filesystem::path p = realPath.string();
            auto absPath = std::filesystem::absolute(p);
            url = "file://" + absPath.string();
        }
        std::string tilesetJson(R"({"Type": "Tileset", "tilesetUrl": ")");
        tilesetJson += url + R"("})";
        auto object = vsgCs::JSONObjectFactory::get()->buildFromSource(tilesetJson);
        auto maybeTileset = vsgCs::ref_ptr_cast<vsgCs::TilesetNode>(object);
        if (!maybeTileset)
        {
            std::cout << "Can't create tileset for " << argString << "\n";
        }
        return maybeTileset;
    }
    vsg::CommandLine& arguments;
    vsg::ref_ptr<vsgCs::RuntimeEnvironment> env;
};

class ViewState
{
public:
    ViewState(vsg::CommandLine& arguments,
                const vsg::ref_ptr<vsgCs::RuntimeEnvironment>& env_,
                const vsg::ref_ptr<vsg::EllipsoidModel>& ellipsoidModel_,
                const vsg::ref_ptr<vsg::Window>& window_)
        : env(env_), ellipsoidModel(ellipsoidModel_), window(window_)
    {
        useEllipsoidPerspective = !arguments.read({"--disble-EllipsoidPerspective", "--dep"});
        while (arguments.read("--poi", poi_latitude, poi_longitude)) {};
        while (arguments.read("--distance", poi_distance)) {};
        // Options for the VSG trackball manipulator
        horizonMountainHeight = arguments.value(0.0, "--hmh");
        maxShadowDistance = arguments.value<double>(10000.0, "--sd");
        twoCameras = arguments.read({"-2"});
    }
     std::vector<vsg::ref_ptr<vsg::View>>& createViews()
    {
        views.clear();
        vsg::dvec3 centre(0.0, 0.0, 0.0);
        double radius = ellipsoidModel->radiusEquator();

        double nearFarRatio = 0.0005;

        // set up the camera
        vsg::ref_ptr<vsg::ProjectionMatrix> perspective;
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
        const VkExtent2D& extent = window->extent2D();
        double aspectRatio = static_cast<double>(extent.width) / extent.height;
        if (twoCameras)
        {
            auto lvp = vsg::ViewportState::create(0, 0, extent.width / 2, extent.height);
            auto rvp = vsg::ViewportState::create(extent.width / 2, 0, extent.width / 2, extent.height);
            // Something big
            double zNear = 50;
            double zFar = radius * 4.0;
            double windowHeight = std::tan(vsg::radians(30.0 / 2.0)) * zNear;
            double windowWidth = windowHeight * aspectRatio;
            auto lproj = CsApp::GeneralPerspective::create(-windowWidth, 0.0,
                                                           -windowHeight, windowHeight,
                                                           zNear, zFar);
            auto rproj = CsApp::GeneralPerspective::create(0.0, windowWidth,
                                                           -windowHeight, windowHeight,
                                                           zNear, zFar);
            views.emplace_back(vsg::View::create(vsg::Camera::create(lproj, lookAt, lvp)));
            views.emplace_back(vsg::View::create(vsg::Camera::create(rproj, lookAt, rvp)));
        }
        else
        {
            if (useEllipsoidPerspective)
            {
                perspective = vsg::EllipsoidPerspective::create(lookAt, ellipsoidModel, 30.0, aspectRatio,
                                                                nearFarRatio, horizonMountainHeight);
            }
            else
            {
                perspective = vsg::Perspective::create(30.0, aspectRatio,
                                                       nearFarRatio * radius, radius * 4.5);
            }
            auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(extent));
            views.emplace_back(vsg::View::create(camera));
        }
        for (auto& view : views)
        {
            view->viewDependentState->maxShadowDistance = maxShadowDistance;
        }
        return views;
    }
    bool setViewpointAfterLoad = false;
    std::vector<vsg::ref_ptr<vsg::View>> views;
    bool useEllipsoidPerspective = true;
    double horizonMountainHeight = 0.0;
    double maxShadowDistance = 10000.0;
    const double invalid_value = std::numeric_limits<double>::max();
    double poi_latitude = invalid_value;
    double poi_longitude = invalid_value;
    double poi_distance = invalid_value;
    bool twoCameras = false;
    vsg::ref_ptr<vsg::LookAt> lookAt;
protected:
    vsg::ref_ptr<vsgCs::RuntimeEnvironment> env;
    vsg::ref_ptr<vsg::EllipsoidModel> ellipsoidModel;
    vsg::ref_ptr<vsg::Window> window;
};
}

int main(int argc, char** argv)
{
    // The default allocator needs to be saved and restored when the
    // program shuts down because static objects' constructors may
    // have made allocations from it. There may be a better place to
    // do this than in main(), but this runs about as early -- and the
    // replacement runs as late -- as any live code can.
#ifdef HAVE_VALGRIND
    std::unique_ptr<vsg::Allocator> savedAllocator;
    if (RUNNING_ON_VALGRIND)
    {
        auto& allocator = vsg::Allocator::instance();
        savedAllocator = std::move(allocator);
        allocator = std::make_unique<vsgCs::CppAllocator>();
    }
    auto restoreAllocator = gsl::finally([&savedAllocator]()
    {
        if (RUNNING_ON_VALGRIND)
        {
            auto& allocator = vsg::Allocator::instance();
            allocator = std::move(savedAllocator);
        }
    });
#endif
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
        environment->options->add(vsgCs::GltfLoader::create(environment));
#ifdef vsgXchange_FOUND
        // add vsgXchange's support for reading and writing 3rd party file formats. If vsgXchange
        // isn't found, then we will only be able to read .vsgt models and glTF files.
        environment->options->add(vsgXchange::all::create());
        arguments.read(environment->options);
#endif
        // numFrames and pathFilename come from vsgViewer. They provide a way to display a
        // "flythrough." At the moment they are not useful because a scene will usually take some
        // time to load.
        auto numFrames = arguments.value(-1, "-f");
        auto pathFilename = arguments.value(std::string(), "-p");
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
#if 0
        auto shadowMaps = arguments.value<uint32_t>(0, "--shadow-maps");
#endif
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
#if 0
            directionalLight->shadowMaps = shadowMaps;
#endif
            vsg_scene->addChild(directionalLight);
        }

        // Do early cesium-native initialization
        vsgCs::startup();
        // create the VSG viewer and assign window(s) to it
        auto viewer = CsApp::CsViewer::create();

        if (!window)
        {
            std::cout << "Could not create windows.\n";
            return 1;
        }
        // VSG's EllipsoidModel provides ellipsoid parameters (e.g. WGS84), mostly for the benefit
        // of the trackball manipulator.
        auto ellipsoidModel = vsg::EllipsoidModel::create();
        // vsgCS::RuntimeEnvironment needs the vsg::Viewer object for creation of Vulkan objects
        environment->setViewer(viewer);
        ViewState viewState(arguments, environment, ellipsoidModel, window);
        VsgCsScenegraphBuilder graphBuilder(arguments, environment);
        auto worldNode = graphBuilder.worldNode;
        auto modelRoot = graphBuilder.xchangeModels;
        worldNode->setObject("EllipsoidModel", ellipsoidModel);
        vsg_scene->addChild(worldNode);
        if (modelRoot)
        {
            vsg_scene->addChild(modelRoot);
        }
        viewer->addWindow(window);
        auto views = viewState.createViews();
        // Basic VSG objects for rendering
        auto commandGraph = vsgCs::TracingCommandGraph::create(environment, window);
        auto renderGraph = vsgCs::TracingRenderGraph::create(window);
        // The classic VSG background, translated into sRGB values.
        renderGraph->setClearValues({{0.02899f, 0.02899f, 0.13321f}});
        commandGraph->addChild(renderGraph);
        for (auto& view : views)
        {
            if (useHeadlight)
            {
                view->addChild(vsg::createHeadlight());
            }
            view->addChild(vsg_scene);
            renderGraph->addChild(view);
        }
        // Create this application's user interface, including the trackball manipulator and the
        // graphical overlay.
        auto ui = vsgCs::UI::create();
        auto uiCamera = views[0]->camera;
        ui->createUI(window, viewer, uiCamera, ellipsoidModel, environment->options, worldNode, vsg_scene,
                     environment, debugManipulator);
        ui->setViewpoint(viewState.lookAt, 0.0);
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
            if (viewState.setViewpointAfterLoad
                && worldNode->getRootTile())
            {
                auto lookAt = vsgCs::makeLookAtFromTile(worldNode->getRootTile(),
                                                        viewState.poi_distance);
                ui->setViewpoint(lookAt, 1.0);
                viewState.setViewpointAfterLoad = false;
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
        std::cerr << "\n[Exception] - " << ve.message << " result = " << ve.result << '\n';
        return 1;
    }

    // clean up done automatically thanks to ref_ptr<>
    // Hah!
    return 0;
}
