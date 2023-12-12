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
#include "ShaderFactory.h"

#include <CesiumGltf/Ktx2TranscodeTargets.h>

#include <vsg/app/CompileManager.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>
#include <vsg/utils/SharedObjects.h>
#include <vsg/vk/Context.h>

namespace vsgCs
{
    /**
     * @brief A compact representation of supported Vulkan features that are important to vsgCs.
     */
    struct VSGCS_EXPORT DeviceFeatures
    {
        bool indexTypeUint8 = false;
        bool largePoints = false;
        bool textureCompressionETC2 = false;
        bool textureCompressionASTC_LDR = false;
        bool textureCompressionBC = false;
        bool textureCompressionPVRTC = false;
        bool depthClamp = false;
        CesiumGltf::Ktx2TranscodeTargets ktx2TranscodeTargets;
        float pointSizeRange[2];
        PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT vkGetPhysicalDeviceCalibrateableTimeDomainsEXT
        = nullptr;
        PFN_vkGetCalibratedTimestampsEXT vkGetCalibratedTimestampsEXT = nullptr;
    };

    class VSGCS_EXPORT GraphicsEnvironment : public vsg::Inherit<vsg::Object, GraphicsEnvironment>
    {
    public:
        GraphicsEnvironment(const vsg::ref_ptr<vsg::Options>& vsgOptions, const DeviceFeatures& in_features,
                            const vsg::ref_ptr<vsg::Device>& in_device);
        /**
         * @brief Run a compile traversal with a minimal context for updating Vulkan handles and such.
         */
        vsg::CompileResult miniCompile(vsg::ref_ptr<vsg::Object> object);
        vsg::ref_ptr<ShaderFactory> shaderFactory;
        const DeviceFeatures features;
        vsg::ref_ptr<vsg::SharedObjects> sharedObjects;
        // XXX If / when multiple devices are supported, this will have to be expanded.
        vsg::ref_ptr<vsg::Device> device;
        /**
         * @brief a white, one pixel texture
         */
        vsg::ref_ptr<vsg::ImageInfo> defaultTexture;
        /**
         * @brief A pipeline layout that is common to all vsgCs tiles through the
         * TILE_DESCRIPTOR_SET set (2). Therefore it can be used to bind descriptor sets 0, 1, and
         * 2.
         */
        vsg::ref_ptr<vsg::PipelineLayout> overlayPipelineLayout;
        vsg::ref_ptr<vsg::ImageInfo> blueNoiseTexture;
    protected:
        vsg::ref_ptr<vsg::CompileTraversal> miniCompileTraversal;
    };

    // Utility
    vsg::ref_ptr<vsg::DescriptorSet> getDescriptorSet(const vsg::ref_ptr<vsg::DescriptorConfigurator>& config,
                                                      unsigned set);
}
