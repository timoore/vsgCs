#include "SamplerData.h"

namespace vsgCesium
{
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
                  return nullptr;
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
                  return nullptr;
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
                  return nullptr;
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
              bool useMipMaps = false;
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


}
