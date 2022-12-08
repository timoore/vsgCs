#include "CesiumGltfBuilder.h"

#include <algorithm>
#include <tuple>

using namespace vsgCesium;
using namespace CesiumGltf;

namespace
{
    bool isGltfIdentity(const std::vector<double>& matrix)
    {
        if (matrix.size() != 16)
            return false;
        for (i = 0; i < 16; ++i)
        {
            if (i % 5 == 0)
            {
                if (matrix[i] != 1.0)
                    return false;
            }
            else
            {
                if (matrix[i] != 0.0)
                    return false;
            }
        }
        return true;
    }
}

CesiumGltfBuilder::CesiumGltfBuilder()
    : _sharedObjects(vsg::SharedObjects::create())
{
}

vsg::ref_ptr<vsg::Group>
CesiumGltfBuilder::ModelBuilder::loadNode(const CesiumGltf::Node* node)
{
    const std::vector<double>& matrix = node.matrix;
    vsg::ref_ptr<vsg::Group> result;
    if (isGltfIdentity(matrix))
    {
        result = vsg::Group::create();
    }
    else
    {
        vsg::dmat4 transformMatrix;
        if (matrix.size() == 16)
        {
            std::copy(matrix.begin(), matrix.end(), transformMatrix.data());
        }
        else
        {
            vsg::dmat4 translation;
            if (node.translation.size() == 3)
            {
                translation = vsg::translate(node.translation[0], node.translation[1], node.translation[2]);
            }
            vsg::dquat rotation(0.0, 0.0, 0.0, 1.0);
            if (node.rotation.size() == 4)
            {
                rotation.x = node.rotation[1];
                rotation.y = node.rotation[2];
                rotation.z = node.rotation[3];
                rotation.w = node.rotation[0];
            }
            vsg::dmat4 scale;
            if (node.scale.size() == 3)
            {
                scale = vsg::scale(node.scale[0], node.scale[1], node.scale[2])
            }
            transformMatrix = translation * rotate(rotation) * scale;
        }
        result = vsg::<vsg::MatrixTransform>::create(transformMatrix);
    }
    int meshId = node.mesh;
    if (meshId >= 0 && meshId < model->meshes.size())
    {
        result->addChild(loadMesh(&model->meshes[meshId]));
    }
    for (int childNodeId : node.children)
    {
        if (childNodeId >= 0 && childNodeId < model->nodes.size())
        {
            result->addChild(loadNode(&model->nodes[childNodeId]));
        }
    }
    return result;
}

vsg::ref_ptr<vsg::Group>
CesiumGltfBuilder::ModelBuilder::operator()()
{
    vsg::ref_ptr<vsg::Group> resultNode = vsg::Group::create();

    if (model->scene >= 0 && model->scene < model->scenes.size())
    {
        // Show the default scene
        const Scene& defaultScene = model->scenes[model->scene];
        for (int nodeId : defaultScene.nodes)
        {
            resultNode->addChild(loadNode(&model->nodes[nodeId]));
        }
    }
    else if (model->scenes.size() > 0)
    {
        // There's no default, so show the first scene
        const Scene& defaultScene = model->scenes[0];
        for (int nodeId : defaultScene.nodes)
        {
            resultNode->addChild(loadNode(&model->nodes[nodeId]));
        }
    }
    else if (model->nodes.size() > 0)
    {
        // No scenes at all, use the first node as the root node.
        resultNode = loadNode(&model->nodes[0]);
    }
    else if (model->meshes.size() > 0)
    {
        // No nodes either, show all the meshes.
        for (const Mesh& mesh : model->meshes)
        {
            resultNode->addChild(loadMesh(&mesh));
        }
    }
    return resultNode;
}

vsg::ref_ptr<vsg::Group>
CesiumGltfBuilder::load(CesiumGltf::Model* model, const CreateModelOptions& options)
{
    ModelBuilder builder(this, CesiumGltf::Model* model, const CreateModelOptions& options);
    return builder();
}

namespace
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
            return sRGB ? VK_FORMAT_UNDEFINED : VK_FORMAT_BC4_UNORM_BLOCK;
        case GpuCompressedPixelFormat::BC5_RG:
            return sRGB ? VK_FORMAT_UNDEFINED : VK_FORMAT_BC5_UNORM_BLOCK;
        case GpuCompressedPixelFormat::BC7_RGBA:
            return sRGB ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;
        case GpuCompressedPixelFormat::ASTC_4x4_RGBA:
            return sRGB ? VK_FORMAT_ASTC_4x4_SRGB_BLOCK : VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case GpuCompressedPixelFormat::PVRTC2_4_RGBA:
            return sRGB ? VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG : VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
        case GpuCompressedPixelFormat::ETC2_EAC_R11:
            return sRGB ? VK_FORMAT_UNDEFINED : VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case GpuCompressedPixelFormat::ETC2_EAC_RG11:
            return sRGB ? VK_FORMAT_UNDEFINED : VK_FORMAT_EAC_RG11_UNORM_BLOCK;
        default:
            // Unsupported compressed texture format.
            return VK_FORMAT_UNDEFINED;
        }
    }

    using BlockSize = std::tuple<uint8_t, uint8_t>;
    BlockSize
    getBlockSize(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:

        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:

        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:

        case VK_FORMAT_BC4_UNORM_BLOCK:

        case VK_FORMAT_BC5_UNORM_BLOCK:

        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:

        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:

        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:

        case VK_FORMAT_EAC_R11_UNORM_BLOCK:

        case VK_FORMAT_EAC_RG11_UNORM_BLOCK:
            return BlockSize(4, 4);
        default:
            return BlockSize(1, 1);
        }
    }

    vsg::ref_ptr<vsg::Data>
    makeArray(uint32_t width, uint32_t height, value_type* data, Properties in_properties)
    {
        switch (in_properties.format)
        {
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8_UNORM:
            return vsg::ubyteArray2D::create(width, height,
                                             reinterpret_cast<vsg::ubyteArray2D::value_type*>(data),
                                             in_properties);
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8_UNORM:
            return vsg::ubvec2Array2D::create(width, height,
                                             reinterpret_cast<vsg::ubyte2Array2D::value_type*>(data),
                                             in_properties);
            
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
            return vsg::ubvec4Array2D::create(width, height,
                                             reinterpret_cast<vsg::ubyte4Array2D::value_type*>(data),
                                             in_properties);
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            return vsg::block64Array2D::create(width, height,
                                               reinterpret_cast<vsg::block64Array2D::value_type*>(data),
                                               in_properties);
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        case VK_FORMAT_EAC_RG11_UNORM_BLOCK:
            return vsg::block128Array2D::create(width, height,
                                                reinterpret_cast<vsg::block128Array2D::value_type*>(data),
                                                in_properties);
        default:
            return {};
        }
    }
    
    void* rgbExpand(CesiumGltf::ImageCesium& image)
    {
        size_t sourceSize = image.pixelData.size();
        size_t destSize = sourceSize * 4 / 3;
        uint8_t* destData
            = new (vsg::allocate(sizeof(uint8_t) * destSize, ALLOCATOR_AFFINITY_DATA)) uint8_t[destSize];
        uint8_t* pDest = destData;
        auto srcItr = image.pixelData.begin();
        while (srcItr != image.pixelData.end())
        {
            for (int i = 0; i < 3; ++i)
            {
                *pDest++ = *srcItr++;
            }
            *pDest++ = 1;
        }
        return destData;
    }
}
SamplerData CesiumGltfBuilder::loadTexture(CesiumTextureSource&& imageSource,
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
    VkFormat pixelFormat = cesiumToVk(image, sRGB);
    if (pixelFormat == VK_FORMAT_UNDEFINED)
    {
        return {};
    }
    SamplerData result;
    vsg::Data::Properties props;
    props.format = pixelFormat;
    props.maxNumMipmaps = image.mipPositions.size();
    std::tie(props.blockWidth, props.blockHeight) = getBlockSize(pixelFormat);
    props.origin = vsg::BOTTOM_LEFT;
    // Assume that the ImageCesium raw format will be fine to upload into Vulkan, except for
    // R8G8B8 uncompressed textures, which are rarely supported.
    void* imageSource = nullptr;
    if (pixelFormat == VK_FORMAT_R8G8B8_UNORM || pixelFormat == VK_FORMAT_R8G8B8_SRGB)
    {
        imageSource = rgbExpand(image);
    }
    else
    {
        uint8_t* destData
            = new (vsg::allocate(sizeof(uint8_t) * destSize, ALLOCATOR_AFFINITY_DATA))
            uint8_t[image.pixelData.size()];
        std::copy(image.pixelData.begin(), image.pixelData.end(), destData);
        imageSource = destData;
    }
    // Assume that there is no advantage in sharing the texture data; might be very false!
    result.data = makeArray(,imageSource, props);
    result.sampler = vsg::Sampler::create();
    result.sampler->addressModeU = getWrapMode(wrapMode[0]);
    result.sampler->addressModeV = getWrapMode(wrapMode[1]);
    result.sampler->addressModeW = getWrapMode(wrapMode[2]);
    result.sampler->anisotropyEnable = VK_TRUE;
    result.sampler->maxAnisotropy = 16.0f;
    result.sampler->maxLod = result.data->properties.maxNumMipmaps;
    result.sampler = _sharedObjects->share(result.sampler);
    return result;
}

SamplerData CesiumGltfBuilder::loadTexture(CesiumGltf::Model& model,
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
