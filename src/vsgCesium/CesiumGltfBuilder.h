#pragma once

#include <vsg/all.h>

#include <CesiumGltf/Model.h>

// Build a VSG scenegraph from a Cesium Gltf Model object.

namespace vsgCesium
{
    struct CreateModelOptions
    {
    };
    
    struct SamplerData
    {
        vsg::ref_ptr<vsg::Sampler> sampler;
        vsg::ref_ptr<vsg::Data> data;
    };

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

    class CesiumGltfBuilder : public vsg::Inherit<vsg::Object, CesiumGltfBuilder>
    {
    public:
        CesiumGltfBuilder();
        class ModelBuilder
        {
        public:
            ModelBuilder(CesiumGltf::Model* model, const CreateModelOptions& options)
                : _model(model), _options(options)
            {
            }
            vsg::ref_ptr<vsg::Group> operator()();
        protected:
            vsg::ref_ptr<vsg::Group> loadNode(const CesiumGltf::Node* node);
        private:
            CesiumGltf::Model* _model;

        };
        friend class ModelBuilder;
        vsg::ref_ptr<vsg::Group> load(CesiumGltf::Model* model, const CreateModelOptions& options);
        SamplerData loadTexture(CesiumGltf::Model& model,
                                const CesiumGltf::Texture& texture,
                                bool sRGB);
        SamplerData loadTexture(CesiumTextureSource&& imageSource,
                                VkSamplerAddressMode addressX,
                                VkSamplerAddressMode addressY,
                                VkFilter minFilter,
                                VkFilter maxFilter,
                                bool useMipMaps,
                                bool sRGB);
    protected:
        vsg::ref_ptr<vsg::SharedObjects> _sharedObjects;
    };

    
    inline void setdmat4(vsg::dmat4& vmat, const glm::dmat4x4& glmmat)
    {
        std::memcpy(vmat.data(), glm::value_ptr(glmmat), sizeof(double) * 16);
    }

    inline vsg::dmat4 glm2vsg(const glm::dmat4x4& glmmat)
    {
        vsg::dmat4 result;
        setdmat4(result, glmmat);
    }
    
    inline bool isIdentity(const glm::dmat4x4& mat)
    {
        for (int c = 0; c < 4; ++c)
        {
            for (int r = 0; r < 4; ++r)
            {
                if (c == r)
                {
                    if (mat[c][r] != 1.0)
                        return false;
                }
                else
                {
                    if (mat[c][r] != 0.0)
                        return false;
                }
            }
        }
        return true;
    }

}
