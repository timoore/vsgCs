#pragma once

// Create sampler and image data from a Cesium texture
#include <vsg/all.h>
#include "CesiumGltf/Model.h"

namespace vsgCesium
{
    struct SamplerData
    {
        vsg::ref_ptr<vsg::Sampler> sampler;
        vsg::ref_ptr<vsg::Data> data;
    };

    SamplerData loadTexture(CesiumGltf::Model& model,
                            const CesiumGltf::Texture& texture,
                            bool sRGB);

    // Alot of hair in Cesium Unreal to support these variants, and it's unclear if any are actually
    // used other than GltfImagePtr


    /**
     * @brief A pointer to a glTF image. This image will be cached and used on the
     * game thread and render thread during texture creation.
     *
     * WARNING: Do not use this form of texture creation if the given pointer will
     * be invalidated before the render-thread texture preparation work is done.
     * XXX Can that happen in VSG? "render thread" textures have safe data.
     */
    struct GltfImagePtr
    {
        CesiumGltf::ImageCesium* pImage;
    };

    /**
     * @brief An index to an image that can be looked up later in the corresponding
     * glTF.
     */
    struct GltfImageIndex
    {
        int32_t index = -1;
        GltfImagePtr resolveImage(const CesiumGltf::Model& model) const;
    };

    /**
     * @brief An embedded image resource.
     */
    struct EmbeddedImageSource
    {
        CesiumGltf::ImageCesium image;
    };

    typedef std::variant<
        GltfImagePtr,
        GltfImageIndex,
        EmbeddedImageSource>
    CesiumTextureSource;

    SamplerData loadTexture(CesiumGltf::Model& model,
                            const CesiumGltf::Texture& texture,
                            bool sRGB);
    SamplerData loadTexture(
        CesiumTextureSource&& imageSource,
        VkSamplerAddressMode addressX,
        VkSamplerAddressMode addressY,
        VkFilter minFilter,
        VkFilter maxFilter,
        bool useMipMaps,
        bool generateMipMaps,
        bool sRGB);
}
