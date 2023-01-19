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
#include <vsg/app/WindowTraits.h>
#include <vsg/core/Inherit.h>
#include <vsg/io/Options.h>

namespace vsgCs
{

    /**
     * @brief A compact representation of supported Vulkan features that are important to vsgCs.
     */
    struct VSGCS_EXPORT DeviceFeatures
    {
        bool indexTypeUint8 = false;
        bool textureCompressionETC2 = false;
        bool textureCompressionASTC_LDR = false;
        bool textureCompressionBC = false;
        bool textureCompressionPVRTC = false;
    };

    /**
     * Objects that are needed by vsgCs for initializing VSG and Vulkan.
     */
    class VSGCS_EXPORT RuntimeEnvironment : public vsg::Inherit<vsg::Object, RuntimeEnvironment>
    {
        public:
        vsg::ref_ptr<vsg::Options> initializeOptions(vsg::CommandLine& arguments, vsg::ref_ptr<vsg::Options> = {});
        vsg::ref_ptr<vsg::WindowTraits> initializeTraits(vsg::CommandLine& arguments,
                                                         vsg::ref_ptr< vsg::WindowTraits> traits = {});
        // Open window without creating a Vulkan logical device
        vsg::ref_ptr<vsg::Window> openSystemWindow(const std::string& name, vsg::ref_ptr<vsg::WindowTraits> traits = {});
        vsg::ref_ptr<vsg::Window> openSystemWindow(vsg::CommandLine& arguments, const std::string& name,
                                                   vsg::ref_ptr<vsg::WindowTraits> traits = {},
                                                   vsg::ref_ptr<vsg::Options> options = {});
                /**
         * @brief Set up the window traits to create the Vulkan Device with the desired features,
         * etc.
         *
         * If the application needs custom Vulkan features and extensions, then it can set up
         * traits->deviceFeatures traits->deviceExtensionNames itself. This function will only touch
         * features and extensions needed for vsgCs.
         * @returns features that are actually available for Cesium and vsgCs.
         */

        DeviceFeatures prepareFeaturesAndExtensions(vsg::ref_ptr<vsg::Window> window);
        // Open window and prepare features and extensions for vsgCs.
        vsg::ref_ptr<vsg::Window> openWindow(const std::string& name, vsg::ref_ptr<vsg::WindowTraits> traits = {});
        vsg::ref_ptr<vsg::Window> openWindow(vsg::CommandLine& arguments, const std::string& name,
                                             vsg::ref_ptr<vsg::WindowTraits> traits = {},
                                                   vsg::ref_ptr<vsg::Options> options = {});
        /** @brief Uusage message for vsg::Options command line parsing.
         */
        static std::string optionsUsage();
        /** @brief Usage message for vsg::WindowTraits command line parsing.
         */
        static std::string traitsUsage();
        /** @brief Usage message for command line options recognized by RuntimeEnvironment.
         */
        static std::string usage();
        vsg::ref_ptr<vsg::Options> options;
        vsg::ref_ptr<vsg::WindowTraits> traits;
        DeviceFeatures features;
        static vsg::ref_ptr<RuntimeEnvironment> get();
    };

    vsg::ref_ptr<RuntimeEnvironment> getRuntimeEnvironment();
}
