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

#include <vsg/all.h>

#include <CesiumGltf/Model.h>
#include <Cesium3DTilesSelection/Tile.h>
#include <glm/gtc/type_ptr.hpp>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorSetLayout.h>

#include "Export.h"
#include "DescriptorSetConfigurator.h"
#include "GraphicsEnvironment.h"
#include "MultisetPipelineConfigurator.h"
#include "RuntimeEnvironment.h"
#include "runtimeSupport.h"


#include <array>

// Build a VSG scenegraph from a Cesium Gltf Model object.

namespace vsgCs
{
    /**
     * @brief Class for representing supported extensions during glTF parsing.
     *
     * Just a sketch for now, but the intent is to represent extensions that are enabled by default,
     * i.e. during 3D Tiles processing, and extensions used by the glTF that we can support.
     */
    class Extension : public vsg::Inherit<vsg::Object, Extension>
    {
    public:
        virtual const std::string& getExtensionName() const = 0;
    };

    template<typename TExtension>
    class OfficialExtension : vsg::Inherit<Extension, OfficialExtension<TExtension>>
    {
    protected:
        static inline std::string _typeName = TExtension::ExtensionName;
    public:
        OfficialExtension(const TExtension* extension = nullptr)
            : gltfExtension(extension)
        {
        }
        const std::string& getExtensionName() const override
        {
            return _typeName;
        }
        const TExtension* gltfExtension;
    };

    // Our "extension" for everything extra in a 3D Tile.
    class Cs3DTilesExtension : public vsg::Inherit<Extension, Cs3DTilesExtension>
    {
    public:
        const std::string& getExtensionName() const override;
    };

    using ExtensionList = std::vector<vsg::ref_ptr<Extension>>;
    /**
     * @brief High-level options for the 3D Tile Builder (called CesiumGltfBuilder at the moment)
     */
    struct CreateModelOptions
    {
        bool renderOverlays = false;
    };
    
    // The functions for attaching and detaching rasters return objects that need to be compiled and
    // may replace objects that should then eventually be freed. I don't think the tile-building
    // commands need this generality.
    struct ModifyRastersResult
    {
        std::vector<vsg::ref_ptr<vsg::Object>> compileObjects;
        std::vector<vsg::ref_ptr<vsg::Object>> deleteObjects;
    };

    // There is a lot of hair in Cesium Unreal to support these variants. It's unclear if any
    // are actually used other than GltfImagePtr

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

    /**
     * @brief Load an image as vsg::Data.
     *
     * This returns vsg::Data because the vsg::Array2D template class does not have a more specific
     * superclass.
     */
    vsg::ref_ptr<vsg::Data> VSGCS_EXPORT loadImage(CesiumGltf::ImageCesium& image, bool useMipMaps, bool sRGB);

    int samplerLOD(const vsg::ref_ptr<vsg::Data>& data, bool generateMipMaps);

    /**
     * @brief create a VSG sampler.
     */
    vsg::ref_ptr<vsg::Sampler> VSGCS_EXPORT makeSampler(VkSamplerAddressMode addressX,
                                                        VkSamplerAddressMode addressY,
                                                        VkFilter minFilter,
                                                        VkFilter maxFilter,
                                                        int maxNumMipMaps);

    class VSGCS_EXPORT CesiumGltfBuilder : public vsg::Inherit<vsg::Object, CesiumGltfBuilder>
    {
    public:
        CesiumGltfBuilder(const vsg::ref_ptr<vsg::Options>& vsgOptions, const DeviceFeatures& deviceFeatures);

        friend class ModelBuilder;
        vsg::ref_ptr<vsg::Group> load(CesiumGltf::Model* model, const CreateModelOptions& options);
        vsg::ref_ptr<vsg::Node> loadTile(Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
                                         const glm::dmat4& transform,
                                         const CreateModelOptions& options);
        vsg::ref_ptr<vsg::Object> attachTileData(Cesium3DTilesSelection::Tile& tile,
                                                 const vsg::ref_ptr<vsg::Node>& node);
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
        vsg::ref_ptr<vsg::ShaderSet> getOrCreatePbrShaderSet(VkPrimitiveTopology topology);

        ModifyRastersResult attachRaster(const Cesium3DTilesSelection::Tile& tile,
                                         const vsg::ref_ptr<vsg::Node>& node,
                                         int32_t overlayTextureCoordinateID,
                                         const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
                                         void* pMainThreadRendererResources,
                                         const glm::dvec2& translation,
                                         const glm::dvec2& scale);
        ModifyRastersResult detachRaster(const Cesium3DTilesSelection::Tile& tile,
                                         const vsg::ref_ptr<vsg::Node>& node,
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
        const DeviceFeatures& getFeatures()
        {
            return _deviceFeatures;
        }
    protected:
        static vsg::ref_ptr<vsg::ImageInfo> makeDefaultTexture();
        vsg::ref_ptr<vsg::SharedObjects> _sharedObjects;
        vsg::ref_ptr<vsg::ShaderSet> _pbrShaderSet;
        vsg::ref_ptr<vsg::ShaderSet> _pbrPointShaderSet;
        vsg::ref_ptr<vsg::Options> _vsgOptions;
        vsg::ref_ptr<vsg::PipelineLayout> _viewParamsPipelineLayout;
        vsg::ref_ptr<vsg::PipelineLayout> _overlayPipelineLayout;
        vsg::ref_ptr<vsg::ImageInfo> _defaultTexture;
        DeviceFeatures _deviceFeatures;
        vsg::ref_ptr<GraphicsEnvironment> _genv;
    };

    // This class should load a standard glTF model, without having builtin support for extensions
    // or our own 3D Tiles cruft. Not there yet...
    class VSGCS_EXPORT ModelBuilder
    {
    public:
    ModelBuilder(const vsg::ref_ptr<GraphicsEnvironment>& genv, CesiumGltf::Model* model, const CreateModelOptions& options,
                     const ExtensionList& enabledExtensions = {});
        vsg::ref_ptr<vsg::Group> operator()();
        vsg::ref_ptr<vsg::Group> loadNode(const CesiumGltf::Node* node);
        vsg::ref_ptr<vsg::Group> loadMesh(const CesiumGltf::Mesh* mesh);
        vsg::ref_ptr<vsg::Node> loadPrimitive(const CesiumGltf::MeshPrimitive* primitive,
                                              const CesiumGltf::Mesh* mesh = nullptr);
        struct CsMaterial;
        vsg::ref_ptr<CsMaterial> loadMaterial(const CesiumGltf::Material* material, VkPrimitiveTopology topology);
        vsg::ref_ptr<CsMaterial> loadMaterial(int i, VkPrimitiveTopology topology);
        vsg::ref_ptr<vsg::Data> loadImage(int i, bool useMipMaps, bool sRGB);
        vsg::ref_ptr<vsg::ImageInfo> loadTexture(const CesiumGltf::Texture& texture, bool sRGB);
        template<typename TI>
        bool loadMaterialTexture(vsg::ref_ptr<CsMaterial> cmat,
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
        std::string makeName(const CesiumGltf::Mesh* mesh, const CesiumGltf::MeshPrimitive* primitive) const;

        vsg::ref_ptr<GraphicsEnvironment>_genv;
        CesiumGltf::Model* _model;
        std::string _name;
        CreateModelOptions _options;
        struct TexInfo
        {
            int coordSet = 0;
        };
        // The VSG objects needed to define a material. So far that is a ShaderSet and the
        // descriptors for one descriptor set. This also includes support for matching glTF textures
        // to specific texture coordinate attributes.
        struct CsMaterial : public vsg::Inherit<vsg::Object, CsMaterial>
        {
            vsg::ref_ptr<DescriptorSetConfigurator> descriptorConfig;
            std::map<std::string, TexInfo> texInfo;
        };
        std::vector<std::array<vsg::ref_ptr<CsMaterial>, 2>> _csMaterials;
        struct ImageData
        {
            vsg::ref_ptr<vsg::Data> image;
            vsg::ref_ptr<vsg::Data> imageWithMipmap;
            bool sRGB = false;
        };
        std::vector<ImageData> _loadedImages;
        vsg::ref_ptr<CsMaterial> _baseMaterial[2];
        template<typename TExtension>
        bool isEnabled() const
        {
            return std::find_if(_activeExtensions.begin(), _activeExtensions.end(),
                                [](const auto& ptr)
                                {
                                    return ref_ptr_cast<TExtension>(ptr) != nullptr;
                                }) != _activeExtensions.end();
        }
        ExtensionList _activeExtensions;
    };
}

