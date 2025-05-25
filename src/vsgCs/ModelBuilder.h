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

#include "vsgCs/Export.h"
#include "GraphicsEnvironment.h"
#include "runtimeSupport.h"

#include <CesiumGltf/TextureInfo.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Logger.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>

#include <array>
#include <memory>
#include <utility>
#include <string>

namespace vsgCs
{
    class Styling;
    class Stylist;

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

    class StylingExtension : public vsg::Inherit<Extension, StylingExtension>
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
        CreateModelOptions(bool in_renderOverlays = false, const vsg::ref_ptr<Styling>& styling = {});
        ~CreateModelOptions();
        bool renderOverlays;
        bool lodFade;
        vsg::ref_ptr<Styling> styling;
    };

    struct TexInfo
    {
        int coordSet = 0;
    };
    // The VSG objects needed to define a material. So far that is a ShaderSet and the
    // descriptors for one descriptor set. This also includes support for matching glTF textures
    // to specific texture coordinate attributes.
    struct CsMaterial : public vsg::Inherit<vsg::Object, CsMaterial>
    {
        vsg::ref_ptr<vsg::DescriptorConfigurator> descriptorConfig;
        std::map<std::string, TexInfo> texInfo;
        // ModelBuilder needs access to some data from CsMaterial, but
        // not much.
        virtual bool hasMap(const std::string &name) const
        {
            return texInfo.contains(name);
        }

        virtual bool doesBlending() const
        {
            return descriptorConfig->blending;
        }

        virtual bool isTwoSided() const
        {
            return descriptorConfig->two_sided;
        }
    };

    // For a model, build all the state necessary to render a glTF
    // primitve. This includes the Vulkan pipeline and a descriptor set
    // (probably). Part of the pipeline state depends on the vertex
    // attributes that are present, so building up the arrays for the
    // VertexIndexDraw and VertexDraw commands is handled here too.

    class ModelStateBuilder;
    
    // Application-wide object for creating state in glTF files of a
    // certain type e.g., 3D Tiles with overlays, generic glTF, or
    // special state needed by a user of vsgCs.
    class GltfStateBuilder : public std::enable_shared_from_this<GltfStateBuilder>
    {
    public:
        static std::shared_ptr<GltfStateBuilder> create();
        virtual std::unique_ptr<ModelStateBuilder> createModelStateBuilder(CesiumGltf::Model* model) = 0;
    };

    class IPrimitiveStateBuilder;
    
    // A state builder for a model, used in creating its scene
    // graph. It might cache state data per glTF material, for example.
    class IModelStateBuilder {
    public:
        virtual ~IModelStateBuilder() = default;
        virtual std::unique_ptr<IPrimitiveStateBuilder> create(const CesiumGltf::MeshPrimitive* primitive) = 0;
    };

    // State builder for rendering an individual primitive.
    class IPrimitiveStateBuilder
    {
    public:
        virtual ~IPrimitiveStateBuilder() = default;
        virtual void assignArray(const std::string& name,
                                 const vsg::ref_ptr<vsg::Data> &array,
                                 VkVertexInputRate vertexInputRate) = 0;
        void assignArray(const std::string& name,
                         const vsg::ref_ptr<vsg::Data>& array) {
            assignArray(name, array, VK_VERTEX_INPUT_RATE_VERTEX);
        }
        virtual void addShaderDefine(std::string symbol) = 0;
        virtual void finalizeState() = 0;
        virtual vsg::DataList &getVertexArrays() = 0;
        virtual vsg::ref_ptr<vsg::StateGroup> getFinalStateGroup() = 0;
        virtual VkPrimitiveTopology getTopology() = 0;
        virtual vsg::ref_ptr<CsMaterial> getMaterial() = 0;

    };

    class PrimitiveStateBuilder;
    class ModelBuilder;

    class ModelStateBuilder : public IModelStateBuilder
    {
    public:
        ModelStateBuilder(ModelBuilder* in_modelBuilder, vsg::ref_ptr<vsgCs::GraphicsEnvironment> in_genv);
        std::unique_ptr<IPrimitiveStateBuilder> create(const CesiumGltf::MeshPrimitive* primitive) override;
        std::vector<std::array<vsg::ref_ptr<CsMaterial>, 2>> _csMaterials;

      protected:
        ModelBuilder *modelBuilder;
        vsg::ref_ptr<vsgCs::GraphicsEnvironment> genv;
    };

    class PrimitiveStateBuilder : public IPrimitiveStateBuilder
    {
    public:
        // WIP constructor until ModelStateBuilder is fleshed out
        PrimitiveStateBuilder(
            vsg::ref_ptr<CsMaterial> in_material,
            VkPrimitiveTopology topology,
            vsg::ref_ptr<vsgCs::GraphicsEnvironment> in_genv);

        void assignArray(const std::string& name,
                         const vsg::ref_ptr<vsg::Data> &array,
                         VkVertexInputRate vertexInputRate) override;
        void addShaderDefine(std::string symbol) override;
        void finalizeState() override;

        vsg::DataList &getVertexArrays() override
        {
            return vertexArrays;
        }

        VkPrimitiveTopology getTopology() override
        {
            return _topology;
        }

        vsg::ref_ptr<CsMaterial> getMaterial() override
        {
            return material;
        }
        vsg::ref_ptr<vsg::StateGroup> getFinalStateGroup() override;

        vsg::ref_ptr<CsMaterial> material;
        vsg::DataList vertexArrays;
        vsg::ref_ptr<vsg::GraphicsPipelineConfigurator> pipelineConf;
        vsg::ref_ptr<vsgCs::GraphicsEnvironment> genv;
    protected:
        VkPrimitiveTopology _topology;
    };

    // This class should load a standard glTF model, without having builtin support for extensions
    // or our own 3D Tiles cruft. Not there yet...
    class VSGCS_EXPORT ModelBuilder
    {
    public:
        ModelBuilder(const vsg::ref_ptr<GraphicsEnvironment> &genv,
                   CesiumGltf::Model *model, const CreateModelOptions &options,
                   ExtensionList enabledExtensions = {});
        ModelBuilder(std::shared_ptr<ModelStateBuilder> modelStateBuilder,
                   CesiumGltf::Model *model, const CreateModelOptions &options,
                     ExtensionList enabledExtensions = {});
        ~ModelBuilder();
        vsg::ref_ptr<vsg::Group> operator()();
        vsg::ref_ptr<vsg::Group> loadNode(const CesiumGltf::Node* node);
        using InstanceData = std::array<vsg::ref_ptr<vsg::vec4Array>, 3>;
        vsg::ref_ptr<vsg::Group> loadMesh(const CesiumGltf::Mesh* mesh,
                                          const InstanceData* instanceData = nullptr);
        vsg::ref_ptr<vsg::Node> loadPrimitive(const CesiumGltf::MeshPrimitive* primitive,
                                              const CesiumGltf::Mesh* mesh = nullptr,
                                              const InstanceData* instanceData = nullptr);
        vsg::ref_ptr<CsMaterial> loadMaterial(const CesiumGltf::Material* material, VkPrimitiveTopology topology);
        vsg::ref_ptr<CsMaterial> loadMaterial(int i, VkPrimitiveTopology topology);
        vsg::ref_ptr<vsg::Data> loadImage(int i, bool useMipMaps, bool sRGB);
        vsg::ref_ptr<vsg::ImageInfo> loadTexture(const CesiumGltf::Texture& texture, bool sRGB);
        bool loadMaterialTexture(vsg::ref_ptr<CsMaterial> cmat,
                                 const std::string& name,
                                 const std::optional<CesiumGltf::TextureInfo>& texInfo,
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
        vsg::ref_ptr<Stylist> _stylist;

      protected:
      std::shared_ptr<IModelStateBuilder> _modelStateBuilder;
    };

    // Helper function for getting an attribute accessor by name.
    const CesiumGltf::Accessor* getAccessor(const CesiumGltf::Model* model,
                                            const CesiumGltf::MeshPrimitive* primitive,
                                            const std::string& name);
}
