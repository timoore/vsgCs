#pragma once

#include <vsg/all.h>

#include <CesiumGltf/Model.h>
#include <Cesium3DTilesSelection/Tile.h>
#include <glm/gtc/type_ptr.hpp>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorSetLayout.h>

#include "Export.h"
#include "DescriptorSetConfigurator.h"

// Build a VSG scenegraph from a Cesium Gltf Model object.

namespace vsgCs
{
    struct CreateModelOptions
    {
        bool renderOverlays = false;
    };
    
    struct SamplerData
    {
        vsg::ref_ptr<vsg::Sampler> sampler;
        vsg::ref_ptr<vsg::Data> data;
    };

    // A lot of hair in Cesium Unreal to support these variants, and it's unclear if any are actually
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

    // Interface from Cesium Native to the VSG scene graph. CesiumGltfBuilder can load Models (glTF
    // assets) and images that are not part of a model.

    class ModelBuilder;

    class VSGCS_EXPORT CesiumGltfBuilder : public vsg::Inherit<vsg::Object, CesiumGltfBuilder>
    {
    public:
        CesiumGltfBuilder(vsg::ref_ptr<vsg::Options> vsgOptions);

        friend class ModelBuilder;
        vsg::ref_ptr<vsg::Group> load(CesiumGltf::Model* model, const CreateModelOptions& options);
        vsg::ref_ptr<vsg::Node> loadTile(Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
                                         const glm::dmat4& transform,
                                         const CreateModelOptions& options);
        vsg::ref_ptr<vsg::ImageInfo> loadTexture(CesiumGltf::ImageCesium& image,
                                                 VkSamplerAddressMode addressX,
                                                 VkSamplerAddressMode addressY,
                                                 VkFilter minFilter,
                                                 VkFilter maxFilter,
                                                 bool useMipMaps,
                                                 bool sRGB);
        vsg::ref_ptr<vsg::ImageInfo> loadTexture(CesiumTextureSource&& imageSource,
                                                 VkSamplerAddressMode addressX,
                                                 VkSamplerAddressMode addressY,
                                                 VkFilter minFilter,
                                                 VkFilter maxFilter,
                                                 bool useMipMaps,
                                                 bool sRGB);
        vsg::ref_ptr<vsg::ShaderSet> getOrCreatePbrShaderSet();
        vsg::ref_ptr<vsg::Command> attachRaster(const Cesium3DTilesSelection::Tile& tile,
                                                vsg::ref_ptr<vsg::Node> node,
                                                int32_t overlayTextureCoordinateID,
                                                const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                                void* pMainThreadRendererResources,
                                                const glm::dvec2& translation,
                                                const glm::dvec2& scale);
        std::pair<vsg::ref_ptr<vsg::Command>, vsg::ref_ptr<vsg::Command>>
            detachRaster(const Cesium3DTilesSelection::Tile& tile,
                         vsg::ref_ptr<vsg::Node> node,
                         int32_t overlayTextureCoordinateID,
                         const Cesium3DTilesSelection::RasterOverlayTile& rasterTile);
        vsg::ref_ptr<vsg::ImageInfo> getDefaultTexture()
        {
            return _defaultTexture;
        }
        vsg::ref_ptr<vsg::PipelineLayout> getViewParamsPipelineLayout()
        {
            return _viewParamsPipelineLayout;
        }
        vsg::ref_ptr<vsg::PipelineLayout> getOverlayPipelineLayout()
        {
            return _overlayPipelineLayout;
        }
    protected:
        static vsg::ref_ptr<vsg::ImageInfo> makeDefaultTexture();
        vsg::ref_ptr<vsg::SharedObjects> _sharedObjects;
        vsg::ref_ptr<vsg::ShaderSet> _pbrShaderSet;
        vsg::ref_ptr<vsg::Options> _vsgOptions;
        vsg::ref_ptr<vsg::PipelineLayout> _viewParamsPipelineLayout;
        vsg::ref_ptr<vsg::PipelineLayout> _overlayPipelineLayout;
        vsg::ref_ptr<vsg::ImageInfo> _defaultTexture;
    };

    class VSGCS_EXPORT ModelBuilder
    {
    public:
        ModelBuilder(CesiumGltfBuilder* builder, CesiumGltf::Model* model, const CreateModelOptions& options);
        vsg::ref_ptr<vsg::Group> operator()();
    protected:
        vsg::ref_ptr<vsg::Group> loadNode(const CesiumGltf::Node* node);
        vsg::ref_ptr<vsg::Group> loadMesh(const CesiumGltf::Mesh* mesh);
        vsg::ref_ptr<vsg::Node> loadPrimitive(const CesiumGltf::MeshPrimitive* primitive,
                                              const CesiumGltf::Mesh* mesh = nullptr);
        struct ConvertedMaterial;
        vsg::ref_ptr<ConvertedMaterial> loadMaterial(const CesiumGltf::Material* material);
        vsg::ref_ptr<ConvertedMaterial> loadMaterial(int i);
        vsg::ref_ptr<vsg::Data> loadImage(int i, bool useMipMaps, bool sRGB);
        vsg::ref_ptr<vsg::ImageInfo> loadTexture(const CesiumGltf::Texture& texture, bool sRGB);
        template<typename TI>
        bool loadMaterialTexture(vsg::ref_ptr<ConvertedMaterial> cmat,
                                 const std::string& name,
                                 const std::optional<TI>& texInfo,
                                 bool sRGB)
        {
            using namespace CesiumGltf;
            if (!texInfo || texInfo.value().index < 0
                || static_cast<unsigned>(texInfo.value().index) >= _model->textures.size())
            {
                if (texInfo && texInfo.value().index >= 0)
                {
                    vsg::warn("Texture index must be less than ", _model->textures.size(),
                              " but is ", texInfo.value().index);
                }
                return false;
            }
            const Texture& texture = _model->textures[texInfo.value().index];
            auto imageInfo = loadTexture(texture, sRGB);
            if (imageInfo)
            {
                cmat->descriptorConfig->assignTexture(name, imageInfo->imageView->image->data, imageInfo->sampler);
                cmat->texInfo.insert({name, TexInfo{static_cast<int>(texInfo.value().texCoord)}});
                return true;
            }
            return false;
        }
        CesiumGltfBuilder* _builder;
        CesiumGltf::Model* _model;
        std::string _name;
        CreateModelOptions _options;
        struct TexInfo
        {
            int coordSet = 0;
        };
        struct ConvertedMaterial : public vsg::Inherit<vsg::Object, ConvertedMaterial>
        {
            vsg::ref_ptr<DescriptorSetConfigurator> descriptorConfig;
            std::map<std::string, TexInfo> texInfo;
        };
        std::vector<vsg::ref_ptr<ConvertedMaterial>> _convertedMaterials;
        struct ImageData
        {
            vsg::ref_ptr<vsg::Data> image;
            vsg::ref_ptr<vsg::Data> imageWithMipmap;
            bool sRGB = false;
        };
        std::vector<ImageData> _loadedImages;
        vsg::ref_ptr<ConvertedMaterial> _defaultMaterial;

    };
}
