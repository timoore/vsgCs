#include "SamplerData.h"

namespace vsgCesium
{
    struct GetImageFromSource
    {
        CesiumGltf::ImageCesium*
        operator()(GltfImagePtr& imagePtr) {
            return imagePtr.pImage;
        }

        CesiumGltf::ImageCesium*
        operator()(EmbeddedImageSource& embeddedImage) {
            return &embeddedImage.image;
        }

        template <typename TSource>
        CesiumGltf::ImageCesium* operator()(TSource& /*source*/) {
            return nullptr;
        }
    };

    VkFormat cesiumToVk(const CesiumGltf::ImageCesium& image, bool sRGB)
    {
        using namespace CesiumGltf;

        VkFormat result = VK_FORMAT_UNDEFINED;
        if (image.compressedPixelFormat == GpuCompressedPixelFormat::NONE)
        {
            switch (image.channels) {
            case 1:
                return sRGB ? VK_FORMAT_R8_SRGB : VK_FORMAT_R8_UNORM;
            case 2:
                return sRGB ? VK_FORMAT_R8G8_SRGB : VK_FORMAT_R8G8_UNORM;
            case 3:
                return sRGB ? VK_FORMAT_R8G8B8_SRGB : VK_FORMAT_R8G8B8_UNORM;
            case 4:
            default:
                return sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
            }
        }
        switch (image.compressedPixelFormat)
        {
        case GpuCompressedPixelFormat::ETC1_RGB:
            // ETC1 is a subset of ETC2
            return sRGB ? VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK : VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case GpuCompressedPixelFormat::ETC2_RGBA:
            return sRGB ? VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK : VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case GpuCompressedPixelFormat::BC1_RGB:
            return sRGB ? VK_FORMAT_BC1_RGB_SRGB_BLOCK : VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case GpuCompressedPixelFormat::BC3_RGBA:
            return sRGB ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
        case GpuCompressedPixelFormat::BC4_R:
            return sRGB ? VK_FORMAT_BC4_SRGB_BLOCK : VK_FORMAT_BC4_UNORM_BLOCK;
        case GpuCompressedPixelFormat::BC5_RG:
            return sRGB ? VK_FORMAT_BC5_SRGB_BLOCK : VK_FORMAT_BC5_UNORM_BLOCK;
        case GpuCompressedPixelFormat::BC7_RGBA:
            return sRGB ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;
        case GpuCompressedPixelFormat::ASTC_4x4_RGBA:
            return sRGB ? VK_FORMAT_ASTC_4x4_SRGB_BLOCK : VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case GpuCompressedPixelFormat::PVRTC2_4_RGBA:
            return sRGB ? VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK : VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK;
        case GpuCompressedPixelFormat::ETC2_EAC_R11:
            return sRGB ? VK_FORMAT_UNDEFINED : VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case GpuCompressedPixelFormat::ETC2_EAC_RG11:
            return sRGB ? VK_FORMAT_UNDEFINED : VK_FORMAT_EAC_RG11_UNORM_BLOCK;
        default:
            // Unsupported compressed texture format.
            return VK_FORMAT_UNDEFINED;
        }
    }

    SamplerData loadTexture(CesiumTextureSource&& imageSource,
                                VkSamplerAddressMode addressX,
                                VkSamplerAddressMode addressY,
                                VkFilter minFilter,
                                VkFilter maxFilter,
                                bool useMipMaps,
                                bool sRGB)
        {
            CesiumGltf::ImageCesium* pImage =
                std::visit(GetImageFromSource{}, imageSource);

            assert(pImage != nullptr);
            CesiumGltf::ImageCesium& image = *pImage;

            if (image.pixelData.empty() || image.width == 0 || image.height == 0)
            {
                return {};
            }
            if (useMipMaps && image.mipPositions.empty())
            {
                std::optional<std::string> errorMsg=
                    CesiumGltfReader::GltfReader::generateMipMaps(image);
                if (errorMessage)
                {
                    vsg::warn(errorMessage);
                }
            }
            VkFormat pixelFormat = cesiumToVk(*pImage, sRGB);
            if (pixelFormat == VK_FORMAT_UNDEFINED)
            {
                return {};
            }
            SamplerData result;
            // Assume that the ImageCesium raw format will be fine to upload into Vulkan, except for
            // R8G8B8 uncompressed textures, which are rarely supported.

            result.sampler = vsg::Sampler::create();
            result.sampler->addressModeU = getWrapMode(wrapMode[0]);
            result.sampler->addressModeV = getWrapMode(wrapMode[1]);
            result.sampler->addressModeW = getWrapMode(wrapMode[2]);
            result.sampler->anisotropyEnable = VK_TRUE;
            result.sampler->maxAnisotropy = 16.0f;
            result.sampler->maxLod = result.data->properties.maxNumMipmaps;

        }

    SamplerData loadTexture(CesiumGltf::Model& model,
                            const CesiumGltf::Texture& texture,
                            bool sRGB)
    {
          const CesiumGltf::ExtensionKhrTextureBasisu* pKtxExtension =
              texture.getExtension<CesiumGltf::ExtensionKhrTextureBasisu>();
          const CesiumGltf::ExtensionTextureWebp* pWebpExtension =
              texture.getExtension<CesiumGltf::ExtensionTextureWebp>();

          int32_t source = -1;
          if (pKtxExtension)
          {
              if (pKtxExtension->source < 0 ||
                  pKtxExtension->source >= model.images.size())
              {
                  vsg::warn("KTX texture source index must be non-negative and less than ",
                            model.images.size(),
                            " but is ",
                            pKtxExtension->source);
                  return {};
              }
              source = pKtxExtension->source;
          }
          else if (pWebpExtension)
          {
              if (pWebpExtension->source < 0 ||
                  pWebpExtension->source >= model.images.size())
              {
                  vsg::warn("WebP texture source index must be non-negative and less than ",
                            model.images.size(),
                            " but is ",
                            pWebpExtension->source);
                  return {};
              }
              source = pWebpExtension->source;
          }
          else
          {
              if (texture.source < 0 || texture.source >= model.images.size())
              {
                  vsg::warn("Texture source index must be non-negative and less than ",
                            model.images.size(),
                            " but is ",
                            texture.source);
                  return SamplerData{};
              }
              source = texture.source;
          }

          CesiumGltf::ImageCesium& image = model.images[source].cesium;
          const CesiumGltf::Sampler* pSampler =
              CesiumGltf::Model::getSafe(&model.samplers, texture.sampler);
          // glTF spec: "When undefined, a sampler with repeat wrapping and auto
          // filtering should be used."
          VkSamplerAddressMode addressX = VK_SAMPLER_ADDRESS_MODE_REPEAT;
          VkSamplerAddressMode addressY = VK_SAMPLER_ADDRESS_MODE_REPEAT;
          VkFilter minFilter = VK_FILTER_LINEAR;
          VkFilter magFilter = VK_FILTER_LINEAR;
          bool useMipMaps = false;

          if (pSampler)
          {
              switch (pSampler->wrapS)
              {
              case CesiumGltf::Sampler::WrapS::CLAMP_TO_EDGE:
                  addressX = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                  break;
              case CesiumGltf::Sampler::WrapS::MIRRORED_REPEAT:
                  addressX = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                  break;
              case CesiumGltf::Sampler::WrapS::REPEAT:
                  addressX = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                  break;
              }
              switch (pSampler->wrapT)
              {
              case CesiumGltf::Sampler::WrapT::CLAMP_TO_EDGE:
                  addressY = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                  break;
              case CesiumGltf::Sampler::WrapT::MIRRORED_REPEAT:
                  addressY = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                  break;
              case CesiumGltf::Sampler::WrapT::REPEAT:
                  addressY = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                  break;
              }
              if (pSampler->magFilter
                  && pSampler->magFilter.value() == CesiumGltf::Sampler::MagFilter::NEAREST)
              {
                  magFilter = VK_FILTER_NEAREST;
              }
              if (pSampler->minFilter)
              {
                  switch (pSampler->minFilter.value()) {
                  case CesiumGltf::Sampler::MinFilter::NEAREST:
                  case CesiumGltf::Sampler::MinFilter::NEAREST_MIPMAP_NEAREST:
                      minFilter = VK_FILTER_NEAREST;
                      break;
                  case CesiumGltf::Sampler::MinFilter::LINEAR:
                  case CesiumGltf::Sampler::MinFilter::LINEAR_MIPMAP_NEAREST:
                  default:
                      break;
                  }
              }
              switch (pSampler->minFilter.value_or(
                          CesiumGltf::Sampler::MinFilter::LINEAR_MIPMAP_LINEAR))
              {
              case CesiumGltf::Sampler::MinFilter::LINEAR_MIPMAP_LINEAR:
              case CesiumGltf::Sampler::MinFilter::LINEAR_MIPMAP_NEAREST:
              case CesiumGltf::Sampler::MinFilter::NEAREST_MIPMAP_LINEAR:
              case CesiumGltf::Sampler::MinFilter::NEAREST_MIPMAP_NEAREST:
                  useMipMaps = true;
                  break;
              default: // LINEAR and NEAREST
                  useMipMaps = false;
                  break;
              }
          }
          return loadTexture(GltfImagePtr{&image},
                             addressX,
                             addressY,
                             minFilter,
                             magFilter,
                             useMipMaps,
                             sRGB);
    }
}
