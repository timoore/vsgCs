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

#include "RuntimeEnvironment.h"
#include "vsgCs/Config.h"
#include <vsg/all.h>
#include <vsgXchange/all.h>

using namespace vsgCs;

vsg::ref_ptr<vsg::Options> RuntimeEnvironment::initializeOptions(vsg::CommandLine &arguments,
                                                                 vsg::ref_ptr<vsg::Options> in_options)
{
    options = in_options;
    if (!options)
    {
        options = vsg::Options::create();
    }
    // We want to support VSG_FILE_PATH, if only for reading vsgCs files (shaders, icons) from a
    // development tree. What if the app doesn't want it? an option?
    vsg::Paths vsgFilePaths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->paths.insert(options->paths.end(), vsgFilePaths.begin(), vsgFilePaths.end());
    options->paths.insert(options->paths.end(), vsg::Path(VSGCS_FULL_DATA_DIR));
    // add vsgXchange's support for reading and writing 3rd party file formats
    options->add(vsgXchange::all::create());
    arguments.read(options);
    uint32_t numOperationThreads = 0;
    if (arguments.read("--ot", numOperationThreads))
    {
        options->operationThreads = vsg::OperationThreads::create(numOperationThreads);
    }
    return options;
}

vsg::ref_ptr<vsg::WindowTraits> RuntimeEnvironment::initializeTraits(vsg::CommandLine& arguments,
                                                                     vsg::ref_ptr< vsg::WindowTraits> in_traits)
{
    traits = in_traits;
    if (!traits)
    {
        traits = vsg::WindowTraits::create();
    }
    traits->debugLayer = arguments.read({"--debug", "-d"});
    traits->apiDumpLayer = arguments.read({"--api", "-a"});
    if (arguments.read("--IMMEDIATE")) traits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    if (arguments.read({"--fullscreen", "--fs"})) traits->fullscreen = true;
    if (arguments.read({"--window", "-w"}, traits->width, traits->height)) { traits->fullscreen = false; }
    arguments.read("--screen", traits->screenNum);
    arguments.read("--display", traits->display);
    arguments.read("--samples", traits->samples);
    traits->swapchainPreferences.surfaceFormat = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    return traits;
}

void RuntimeEnvironment::initializeCs(vsg::CommandLine& arguments)
{
    ionAccessToken = arguments.value(std::string(), "--ion-token");
    auto ionTokenFile = arguments.value(std::string(), "--ion-token-file");
    if (!ionAccessToken.empty() && !ionTokenFile.empty())
    {
        vsg::warn("--ion-token and --ion-token-file both specified; using --ion-token");
    }
    if (ionAccessToken.empty() && !ionTokenFile.empty())
    {
        // Slurp it in.
        std::ifstream infile(ionTokenFile);
        if (!infile || !std::getline(infile, ionAccessToken))
        {
            vsg::fatal("Can't read ion token file ", ionTokenFile);
        }
    }
}

void RuntimeEnvironment::initialize(vsg::CommandLine &arguments,
                                    vsg::ref_ptr<vsg::WindowTraits> in_traits,
                                    vsg::ref_ptr<vsg::Options> in_options)
{
    initializeOptions(arguments, in_options);
    initializeTraits(arguments, in_traits);
    initializeCs(arguments);
}

vsg::ref_ptr<vsg::Window> RuntimeEnvironment::openSystemWindow(const std::string& name,
                                                               vsg::ref_ptr<vsg::WindowTraits> in_traits,
                                                               vsg::ref_ptr<vsg::Options> in_options)
{
    if (in_traits)
    {
        traits = in_traits;
    }
    if (in_options)
    {
        options = in_options;
    }
    traits->windowTitle = name;
    return vsg::Window::create(traits);
}

vsg::ref_ptr<vsg::Window> RuntimeEnvironment::openSystemWindow(vsg::CommandLine& arguments, const std::string& name,
                                                               vsg::ref_ptr<vsg::WindowTraits> in_traits,
                                                               vsg::ref_ptr<vsg::Options> in_options)
{
    initialize(arguments, in_traits, in_options);
    return openSystemWindow(name);
}

DeviceFeatures RuntimeEnvironment::prepareFeaturesAndExtensions(vsg::ref_ptr<vsg::Window> window)
{
    if (window->getDevice())
    {
        vsg::warn("The Vulkan Device has already been created; features may or may not be enabled.");
    }
    if (traits != window->traits())
    {
        vsg::warn("Window traits are different, wtf");
    }
    if (!traits->deviceFeatures)
    {
        traits->deviceFeatures = vsg::DeviceFeatures::create();
    }
    auto physDevice = window->getOrCreatePhysicalDevice();
    auto indexFeature
        = physDevice->getFeatures<VkPhysicalDeviceIndexTypeUint8FeaturesEXT,
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT>();
    if (indexFeature.indexTypeUint8 == 1)
    {
        traits->deviceExtensionNames.push_back(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME);
        auto& indexFeatures
            = traits->deviceFeatures->get<VkPhysicalDeviceIndexTypeUint8FeaturesEXT,
                                          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT>();
        indexFeatures.indexTypeUint8 = 1;
        features.indexTypeUint8 = true;
    }
    const auto& physFeatures = physDevice->getFeatures();
    if (physFeatures.textureCompressionETC2)
    {
        features.textureCompressionETC2 = true;
        traits->deviceFeatures->get().textureCompressionETC2 = 1;
    }
    if (physFeatures.textureCompressionASTC_LDR)
    {
        features.textureCompressionASTC_LDR = true;
        traits->deviceFeatures->get().textureCompressionASTC_LDR = 1;
    }
    if (physFeatures.textureCompressionBC)
    {
        features.textureCompressionBC = true;
        traits->deviceFeatures->get().textureCompressionBC = 1;
    }
    auto extensionProperties = physDevice->enumerateDeviceExtensionProperties();
    for (VkExtensionProperties extension : extensionProperties)
    {
        if (!strcmp(extension.extensionName, VK_IMG_FORMAT_PVRTC_EXTENSION_NAME))
        {
            features.textureCompressionPVRTC = true;
            traits->deviceExtensionNames.push_back(VK_IMG_FORMAT_PVRTC_EXTENSION_NAME);
        }
    }
    CesiumGltf::SupportedGpuCompressedPixelFormats supportedFormats;
    if (features.textureCompressionETC2)
    {
        supportedFormats.ETC1_RGB = true;
        supportedFormats.ETC2_RGBA = true;
        supportedFormats.ETC2_EAC_R11 = true;
        supportedFormats.ETC2_EAC_RG11 = true;
    }
    if (features.textureCompressionBC)
    {
        supportedFormats.BC1_RGB = true;
        supportedFormats.BC3_RGBA = true;
        supportedFormats.BC4_R = true;
        supportedFormats.BC5_RG = true;
        supportedFormats.BC7_RGBA = true;
    }
    if (features.textureCompressionASTC_LDR)
    {
        supportedFormats.ASTC_4x4_RGBA = true;
    }
    if (features.textureCompressionPVRTC)
    {
        supportedFormats.PVRTC2_4_RGBA = true;
    }
    features.ktx2TranscodeTargets = CesiumGltf::Ktx2TranscodeTargets(supportedFormats, false);
    return features;
}

vsg::ref_ptr<vsg::Window> RuntimeEnvironment::openWindow(const std::string& name,
                                                         vsg::ref_ptr<vsg::WindowTraits> in_traits,
                                                         vsg::ref_ptr<vsg::Options> in_options)
{
    auto result = openSystemWindow(name, in_traits, in_options);
    prepareFeaturesAndExtensions(result);
    return result;
}

vsg::ref_ptr<vsg::Window> RuntimeEnvironment::openWindow(vsg::CommandLine& arguments, const std::string& name,
                                                         vsg::ref_ptr<vsg::WindowTraits> in_traits,
                                                         vsg::ref_ptr<vsg::Options> in_options)
{
    auto result = openSystemWindow(arguments, name, in_traits, in_options);
    prepareFeaturesAndExtensions(result);
    return result;
    
}

vsg::ref_ptr<RuntimeEnvironment> RuntimeEnvironment::get()
{
    static auto runtimeEnvironment = RuntimeEnvironment::create();
    return runtimeEnvironment;
}

std::string RuntimeEnvironment::optionsUsage()
{
    return std::string(
        "--file-cache path\t VSG file cache\n"
        "--ot numThreads\t\t number of operation threads\n");
}

std::string RuntimeEnvironment::traitsUsage()
{
    return std::string(
        "--debug|-d\t\t load Vulkan debug layer\n"
        "--api|-a\t\t load Vulkan dump layer\n"
        "--IMMEDIATE\t\t set swapchain present mode to VK_PRESENT_MODE_IMMEDIATE_KHR\n"
        "--fullscreen|--fs\t fullscreen window\n"
        "--window|-w width height set window dimensions\n"
        "--screen number\n"
        "--display number\n"
        "--samples n\t\t enable multisamples with n samples\n");
}

std::string RuntimeEnvironment::csUsage()
{
    return std::string(
        "--ion-token token_string user's Cesium ion token\n"
        "--ion-token-file filename file containing user's ion token\n");
}

std::string RuntimeEnvironment::usage()
{
    return optionsUsage() + traitsUsage() + csUsage();
}
