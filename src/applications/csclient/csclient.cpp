

#include <vsg/all.h>

#include <vsgXchange/all.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "vsgCs/TilesetNode.h"

int main(int argc, char** argv)
{
    try
    {
        // set up defaults and read command line arguments to override them
        vsg::CommandLine arguments(&argc, argv);

        // set up vsg::Options to pass in filepaths and ReaderWriter's and other IO related options to use when reading and writing files.
        auto options = vsg::Options::create();
        options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
        options->paths = vsg::getEnvPaths("VSG_FILE_PATH");

        // add vsgXchange's support for reading and writing 3rd party file formats
        options->add(vsgXchange::all::create());

        arguments.read(options);

        auto windowTraits = vsg::WindowTraits::create();
        windowTraits->windowTitle = "csclient";
        windowTraits->debugLayer = arguments.read({"--debug", "-d"});
        windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});
        if (arguments.read("--IMMEDIATE")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        if (arguments.read({"--fullscreen", "--fs"})) windowTraits->fullscreen = true;
        if (arguments.read({"--window", "-w"}, windowTraits->width, windowTraits->height)) { windowTraits->fullscreen = false; }
        arguments.read("--screen", windowTraits->screenNum);
        arguments.read("--display", windowTraits->display);
        arguments.read("--samples", windowTraits->samples);
        auto numFrames = arguments.value(-1, "-f");
        auto pathFilename = arguments.value(std::string(), "-p");
        auto horizonMountainHeight = arguments.value(0.0, "--hmh");
        bool useEllipsoidPerspective = !arguments.read({"--disble-EllipsoidPerspective", "--dep"});
        arguments.read("--file-cache", options->fileCache);
        bool osgEarthStyleMouseButtons = arguments.read({"--osgearth", "-e"});

        uint32_t numOperationThreads = 0;
        if (arguments.read("--ot", numOperationThreads)) options->operationThreads = vsg::OperationThreads::create(numOperationThreads);

        const double invalid_value = std::numeric_limits<double>::max();
        double poi_latitude = invalid_value;
        double poi_longitude = invalid_value;
        double poi_distance = invalid_value;
        while (arguments.read("--poi", poi_latitude, poi_longitude)) {};
        while (arguments.read("--distance", poi_distance)) {};
        if (int log_level = 0; arguments.read("--log-level", log_level))
        {
            vsg::Logger::instance()->level = vsg::Logger::Level(log_level);
        }
        int64_t ionAsset = arguments.value<int64_t>(-1L, "--ionAsset");
        auto ionToken = arguments.value(std::string(), "--ion-token");
        auto tilesetUrl = arguments.value(std::string(), "--tileset-url");
        auto ionEndpointUrl = arguments.value.(std::string(), --ion-endpoint-url);
        if (arguments.errors()) return arguments.writeErrorMessages(std::cerr);

        auto group = vsg::Group::create();
        vsg::Path path;

        // read any vsg files
        for (int i = 1; i < argc; ++i)
        {
            vsg::Path filename = arguments[i];
            path = vsg::filePath(filename);

            auto object = vsg::read(filename, options);
            if (auto node = object.cast<vsg::Node>(); node)
            {
                group->addChild(node);
            }
            else if (auto data = object.cast<vsg::Data>(); data)
            {
                if (auto textureGeometry = createTextureQuad(data, mipmapLevelsHint); textureGeometry)
                {
                    group->addChild(textureGeometry);
                }
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
        vsg::ref_ptr<vsg::Node> vsg_scene = group;
        
        auto window = vsg::Window::create(windowTraits);
        auto deviceFeatures = vsgCs::TilesetNode::prepareDeviceFeatures(window);
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
            source.ionAccessToken = ionToken;
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
        Cesium3DTilesSelction::TilesetOptions& options;
        auto tilesetNode = vsgCs::TilesetNode::create(deviceFeatures, source, options);
        auto ellipsoidModel = vsg::EllipsoidModel::create();
        tilesetNode->setObject("EllipsoidModel", ellipsoidModel);
        vsg_scene->addChild(tilesetNode);
        viewer->addWindow(window);

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
        if (ellipsoidModel)
        {
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
                lookAt = vsg::LookAt::create(centre + vsg::dvec3(-radius * 3.5, 0.0, 0.0), centre, vsg::dvec3(0.0, 1.0, 0.0));
            }

            if (useEllipsoidPerspective)
            {
                perspective = vsg::EllipsoidPerspective::create(lookAt, ellipsoidModel, 30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio, horizonMountainHeight);
            }
            else
            {
                perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio * radius, radius * 4.5);
            }
        }
        else
        {
            perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio * radius, radius * 4.5);
            lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, -radius * 3.5, 0.0), centre, vsg::dvec3(0.0, 0.0, 1.0));
        }

        auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));

        // add close handler to respond the close window button and pressing escape
        viewer->addEventHandler(vsg::CloseHandler::create(viewer));

        if (pathFilename.empty())
        {
            if (ellipsoidModel)
            {
                auto trackball = vsg::Trackball::create(camera, ellipsoidModel);
                trackball->addKeyViewpoint(vsg::KeySymbol('1'), 51.50151088842245, -0.14181489107549874, 2000.0, 2.0); // Grenwish Observatory
                trackball->addKeyViewpoint(vsg::KeySymbol('2'), 55.948642740309324, -3.199226855522667, 2000.0, 2.0);  // Edinburgh Castle
                trackball->addKeyViewpoint(vsg::KeySymbol('3'), 48.858264952330764, 2.2945039609604665, 2000.0, 2.0);  // Eiffel Town, Paris
                trackball->addKeyViewpoint(vsg::KeySymbol('4'), 52.5162603714634, 13.377684902745642, 2000.0, 2.0);    // Brandenburg Gate, Berlin
                trackball->addKeyViewpoint(vsg::KeySymbol('5'), 30.047448591298807, 31.236319571791213, 10000.0, 2.0); // Cairo
                trackball->addKeyViewpoint(vsg::KeySymbol('6'), 35.653099536061156, 139.74704060056993, 10000.0, 2.0); // Tokyo
                trackball->addKeyViewpoint(vsg::KeySymbol('7'), 37.38701052699002, -122.08555895549424, 10000.0, 2.0); // Mountain View, California
                trackball->addKeyViewpoint(vsg::KeySymbol('8'), 40.689618207006355, -74.04465595488215, 10000.0, 2.0); // Empire State Building
                trackball->addKeyViewpoint(vsg::KeySymbol('9'), 25.997055873649554, -97.15543476551771, 1000.0, 2.0);  // Boca Chica, Taxas

                if (osgEarthStyleMouseButtons)
                {
                    trackball->panButtonMask = vsg::BUTTON_MASK_1;
                    trackball->rotateButtonMask = vsg::BUTTON_MASK_2;
                    trackball->zoomButtonMask = vsg::BUTTON_MASK_3;
                }

                viewer->addEventHandler(trackball);
            }
            else
            {
                viewer->addEventHandler(vsg::Trackball::create(camera));
            }
        }
        else
        {
            auto animationPath = vsg::read_cast<vsg::AnimationPath>(pathFilename, options);
            if (!animationPath)
            {
                std::cout<<"Warning: unable to read animation path : "<<pathFilename<<std::endl;
                return 1;
            }
            viewer->addEventHandler(vsg::AnimationPathHandler::create(camera, animationPath, viewer->start_point()));
        }

        auto commandGraph = vsg::createCommandGraphForView(window, camera, vsg_scene);
        viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});
        viewer->compile();

        tilesetNode->initialize(viewer);
        
        // rendering main loop
        while (viewer->advanceToNextFrame() && (numFrames < 0 || (numFrames--) > 0))
        {
            // pass any events into EventHandlers assigned to the Viewer
            viewer->handleEvents();

            viewer->update();

            viewer->recordAndSubmit();

            viewer->present();
        }

        {
            std::scoped_lock<std::mutex> lock(tileReader->statsMutex);
            std::cout << "numOperationThreads = " << numOperationThreads << std::endl;
            std::cout << "numTilesRead = " << tileReader->numTilesRead << std::endl;
            std::cout << "average TimeReadingTiles = " << (tileReader->totalTimeReadingTiles / static_cast<double>(tileReader->numTilesRead)) << std::endl;
        }
    }
    catch (const vsg::Exception& ve)
    {
        for (int i = 0; i < argc; ++i) std::cerr << argv[i] << " ";
        std::cerr << "\n[Exception] - " << ve.message << " result = " << ve.result << std::endl;
        return 1;
    }

    // clean up done automatically thanks to ref_ptr<>
    return 0;
}
