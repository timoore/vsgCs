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

#pragma once

#include "Export.h"
#include "GraphicsEnvironment.h"
#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/core/Inherit.h>
#include <vsg/io/Options.h>

namespace vsgCs
{

    class TracyContextValue;

    /**
     * Objects that are needed by vsgCs for initializing VSG, Vulkan, Cesium Ion...
     */
    class VSGCS_EXPORT RuntimeEnvironment : public vsg::Inherit<vsg::Object, RuntimeEnvironment>
    {
    public:
        RuntimeEnvironment();
        vsg::ref_ptr<vsg::Options> initializeOptions(vsg::CommandLine& arguments,
                                                     const vsg::ref_ptr<vsg::Options>& options= {});
        vsg::ref_ptr<vsg::WindowTraits> initializeTraits(vsg::CommandLine& arguments,
                                                         const vsg::ref_ptr< vsg::WindowTraits>& traits = {});
        void initializeCs(vsg::CommandLine& arguments);
        // Parse all command line arguments and initalize everything
        void initialize(vsg::CommandLine& arguments,
                        const vsg::ref_ptr<vsg::WindowTraits>& traits = {},
                        const vsg::ref_ptr<vsg::Options>& options = {});
        // Open window without creating a Vulkan logical device
        vsg::ref_ptr<vsg::Window> openSystemWindow(const std::string& name,
                                                   const vsg::ref_ptr<vsg::WindowTraits>& traits = {},
                                                   const vsg::ref_ptr<vsg::Options>& options = {});
        vsg::ref_ptr<vsg::Window> openSystemWindow(vsg::CommandLine& arguments, const std::string& name,
                                                   const vsg::ref_ptr<vsg::WindowTraits>& traits = {},
                                                   const vsg::ref_ptr<vsg::Options>& options = {});
        /**
         * @brief Set up the window traits to create the Vulkan Device with the desired features,
         * etc.
         *
         * If the application needs custom Vulkan features and extensions, then it can set up
         * traits->deviceFeatures and traits->deviceExtensionNames itself. This function will only touch
         * features and extensions needed for vsgCs.
         * @returns features that are actually available for Cesium and vsgCs.
         */

        DeviceFeatures prepareFeaturesAndExtensions(const vsg::ref_ptr<vsg::Window>& window);

        /**
         * @brief Initialize the graphics environment object. Not called by client code unless
         * it is doing its own intitialization of the RuntimeEnvironment.
         */
        void initGraphicsEnvironment(const vsg::ref_ptr<vsg::Device>& device);
        
        // Open window and prepare features and extensions for vsgCs.
        vsg::ref_ptr<vsg::Window> openWindow(const std::string& name,
                                             const vsg::ref_ptr<vsg::WindowTraits>& traits = {},
                                             const vsg::ref_ptr<vsg::Options>& options = {});
        vsg::ref_ptr<vsg::Window> openWindow(vsg::CommandLine& arguments, const std::string& name,
                                             const vsg::ref_ptr<vsg::WindowTraits>& traits = {},
                                             const vsg::ref_ptr<vsg::Options>& options = {});

        /**
         * Prepare the window traits / features / extensions for an existing window and initialize
         * environment.
         */
        void initializeFromWindow(const vsg::ref_ptr<vsg::Window>& window,
                                  const vsg::ref_ptr<vsg::Options>& options = {});

        /**
         * @brief Set the viewer object.
         *
         * TilesetNode and WorldNode objects can be created before the VSG viewer, but
         * RuntimeEnvironment needs the viewer before code can start running.
         */
        void setViewer(const vsg::ref_ptr<vsg::Viewer>& viewer);
        
        /**
         * Get the tileset externals that should be used by every tileset.
         */
        std::shared_ptr<Cesium3DTilesSelection::TilesetExternals> getTilesetExternals();

        std::shared_ptr<CesiumAsync::IAssetAccessor> getAssetAccessor()
        {
            return getTilesetExternals()->pAssetAccessor;
        }

        vsg::ref_ptr<vsg::Viewer> getViewer();

        /**
         * @brief Update the environment for a new frame.
         *
         * XXX This should be in a viewer operation.
         */
        void update();
         
        /** @brief Uusage message for vsg::Options command line parsing.
         */
        static std::string optionsUsage();
        
        /** @brief Usage message for vsg::WindowTraits command line parsing.
         */
        static std::string traitsUsage();
        
        /** @brief Usage message for vsgCs command line parsing.
         */
        static std::string csUsage();
        
        /** @brief Usage message for command line options recognized by RuntimeEnvironment.
         */
        static std::string usage();
        vsg::ref_ptr<vsg::Options> options;
        vsg::ref_ptr<vsg::WindowTraits> traits;
        DeviceFeatures features;
        std::string ionAccessToken;
        bool generateShaderDebugInfo = false;
        bool enableLodTransitionPeriod = false;
        vsg::ref_ptr<GraphicsEnvironment> genv;
        vsg::ref_ptr<TracyContextValue> tracyContext;
        static vsg::ref_ptr<RuntimeEnvironment> get();
    protected:
        std::shared_ptr<Cesium3DTilesSelection::TilesetExternals> _externals;
        std::optional<std::string> _csCacheFile;
    };
}
