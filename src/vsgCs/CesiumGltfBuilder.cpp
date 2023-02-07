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

#include "CesiumGltfBuilder.h"
#include "pbr.h"
#include "DescriptorSetConfigurator.h"
#include "MultisetPipelineConfigurator.h"
#include "LoadGltfResult.h"
#include "runtimeSupport.h"

// Some include file in Cesium (actually, it's spdlog.h) unleashes the hell of windows.h. We need to
// turn off GDI defines to avoid a redefinition of the GLSL constant OPAQUE.
#ifndef NOGDI
#define NOGDI
#endif

#include <CesiumGltf/AccessorView.h>
#include <CesiumGltf/ExtensionKhrTextureBasisu.h>
#include <CesiumGltf/ExtensionTextureWebp.h>

#include <CesiumGltfReader/GltfReader.h>
#include <Cesium3DTilesSelection/GltfUtilities.h>
#include <Cesium3DTilesSelection/RasterOverlayTile.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>

#include <algorithm>
#include <string>
#include <tuple>
#include <limits>
#include <exception>
#include <vsg/utils/ShaderSet.h>

using namespace vsgCs;
using namespace CesiumGltf;

namespace
{
    bool isGltfIdentity(const std::vector<double>& matrix)
    {
        if (matrix.size() != 16)
            return false;
        for (int i = 0; i < 16; ++i)
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

    // From Cesium Unreal
    /**
     * @brief Constrain the length of the given string.
     *
     * If the string is shorter than the maximum length, it is returned.
     * If it is not longer than 3 characters, the first maxLength
     * characters will be returned.
     * Otherwise, the result will be of the form `prefix + "..." + suffix`,
     * with the prefix and suffix chosen so that the length of the result
     * is maxLength
     *
     * @param s The input string
     * @param maxLength The maximum length.
     * @return The constrained string
     */
    std::string constrainLength(const std::string& s, const size_t maxLength)
    {
        if (s.length() <= maxLength)
        {
            return s;
        }
        if (maxLength <= 3)
        {
            return s.substr(0, maxLength);
        }
        const std::string ellipsis("...");
        const size_t prefixLength = ((maxLength - ellipsis.length()) + 1) / 2;
        const size_t suffixLength = (maxLength - ellipsis.length()) / 2;
        const std::string prefix = s.substr(0, prefixLength);
        const std::string suffix = s.substr(s.length() - suffixLength, suffixLength);
        return prefix + ellipsis + suffix;
    }

    bool generateNormals(vsg::ref_ptr<vsg::vec3Array> positions, vsg::ref_ptr<vsg::vec3Array> normals,
                         VkPrimitiveTopology topology)
    {
        auto setNormals =
            [&](uint32_t p0, uint32_t p1, uint32_t p2)
            {
                vsg::vec3 v0 = (*positions)[p1] - (*positions)[p0];
                vsg::vec3 v1 = (*positions)[p2] - (*positions)[p0];
                vsg::vec3 perp = vsg::cross(v0, v1);
                float len = vsg::length(perp);
                if (len > 0.0f)
                {
                    perp = perp / len;
                }
                else
                {
                    // The edges are parallel and the triangle is degenerate. Try to construct
                    // something perpendicular to the edges.
                    perp = vsg::vec3(-v0.y, v0.x, 0.0);
                    len = vsg::length(perp);
                    if (len > 0.0f)
                    {
                        perp = perp / len;
                    }
                    else
                    {
                        perp = vsg::vec3(0.0f, 1.0f, 0.0f);
                    }
                }
                (*normals)[p0] = perp;
                if (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                {
                    (*normals)[p1] = perp;
                    (*normals)[p2] = perp;
                }
            };
        switch (topology)
        {
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
            mapTriangleList(static_cast<uint32_t>(positions->size()), setNormals);
            return true;
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
            mapTriangleStrip(uint32_t(positions->size()), setNormals);
            return true;
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
            mapTriangleFan(uint32_t(positions->size()), setNormals);
            return true;
        default:
            return false;
        }
    }
}

inline VkDescriptorSetLayoutBinding getVk(const vsg::UniformBinding& binding)
{
    return VkDescriptorSetLayoutBinding{binding.binding, binding.descriptorType, binding.descriptorCount,
                                        binding.stageFlags, nullptr};
}

CesiumGltfBuilder::CesiumGltfBuilder(vsg::ref_ptr<vsg::Options> vsgOptions,
                                     const DeviceFeatures& deviceFeatures)
    : _sharedObjects(create_or<vsg::SharedObjects>(vsgOptions->sharedObjects)),
      _pbrShaderSet(pbr::makeShaderSet(vsgOptions)),
      _pbrPointShaderSet(pbr::makePointShaderSet(vsgOptions)),
      _vsgOptions(vsgOptions),
      _deviceFeatures(deviceFeatures)
{
    std::set<std::string> shaderDefines;
    shaderDefines.insert("VSG_TWO_SIDED_LIGHTING");
    shaderDefines.insert("VSGCS_OVERLAY_MAPS");
    _viewParamsPipelineLayout = makePipelineLayout(_pbrShaderSet, shaderDefines, pbr::VIEW_DESCRIPTOR_SET);
    _overlayPipelineLayout = makePipelineLayout(_pbrShaderSet, shaderDefines, pbr::TILE_DESCRIPTOR_SET);
    _defaultTexture = makeDefaultTexture();
}

vsg::ref_ptr<vsg::ShaderSet> CesiumGltfBuilder::getOrCreatePbrShaderSet(VkPrimitiveTopology topology)
{
    if (topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
    {
        return _pbrPointShaderSet;
    }
    else
    {
        return _pbrShaderSet;
    }
}

ModelBuilder::ModelBuilder(CesiumGltfBuilder* builder, CesiumGltf::Model* model,
                           const CreateModelOptions& options)
    : _builder(builder), _model(model), _options(options),
      _convertedMaterials(model->materials.size()),
      _loadedImages(model->images.size())
{
    _name = "glTF";
    auto urlIt = _model->extras.find("Cesium3DTiles_TileUrl");
    if (urlIt != _model->extras.end())
    {
        _name = urlIt->second.getStringOrDefault("glTF");
        _name = constrainLength(_name, 256);
    }
}

vsg::ref_ptr<vsg::Group>
ModelBuilder::loadNode(const CesiumGltf::Node* node)
{
    const std::vector<double>& matrix = node->matrix;
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
            if (node->translation.size() == 3)
            {
                translation = vsg::translate(node->translation[0], node->translation[1], node->translation[2]);
            }
            vsg::dquat rotation(0.0, 0.0, 0.0, 1.0);
            if (node->rotation.size() == 4)
            {
                rotation.x = node->rotation[1];
                rotation.y = node->rotation[2];
                rotation.z = node->rotation[3];
                rotation.w = node->rotation[0];
            }
            vsg::dmat4 scale;
            if (node->scale.size() == 3)
            {
                scale = vsg::scale(node->scale[0], node->scale[1], node->scale[2]);
            }
            transformMatrix = translation * rotate(rotation) * scale;
        }
        result = vsg::MatrixTransform::create(transformMatrix);
    }
    int meshId = node->mesh;
    if (meshId >= 0 && static_cast<unsigned>(meshId) < _model->meshes.size())
    {
        result->addChild(loadMesh(&_model->meshes[meshId]));
    }
    for (int childNodeId : node->children)
    {
        if (childNodeId >= 0 && static_cast<unsigned>(childNodeId) < _model->nodes.size())
        {
            result->addChild(loadNode(&_model->nodes[childNodeId]));
        }
    }
    return result;
}

vsg::ref_ptr<vsg::Group>
ModelBuilder::loadMesh(const CesiumGltf::Mesh* mesh)
{
    auto result = vsg::Group::create();
    int primNum = 0;
    try
    {
        for (const CesiumGltf::MeshPrimitive& primitive : mesh->primitives)
        {
            result->addChild(loadPrimitive(&primitive, mesh));
            ++primNum;
        }
    }
    catch (std::runtime_error& err)
    {
        vsg::warn("Error loading mesh, prim ", primNum, err.what());
    }
    return result;
}

// Copying vertex attributes
// The shader set specifies the attribute format as a VkFormat. Either we supply the data in that
// format, or we have to set the format property of the vsg::Array. The default pbr shader requires
// all its attributes in float format. For now we will convert the data to float, but eventually we
// will want to supply the data in the more compact formats, if they are supported.
//
// The PBR shader expects color data as RGBA, but that is just too nasty! Set the correct format
// for RGB if that is provided by the glTF asset.

namespace
{
    // Helpers for writing templates that take Cesium Accessors as arguments.

    template<typename T> struct AccessorViewTraits;

    template<template<typename> typename  VSGType, typename TValue>
    class VSGTraits
    {
    public:
        using element_type = typename VSGType<TValue>::value_type;
        using value_type = VSGType<TValue>;
        using array_type = vsg::Array<value_type>;
        static constexpr std::size_t size = VSGType<TValue>().size();
        template<typename TOther>
        using with_element_type = VSGType<TOther>;
    };

    template<typename T>
    struct AccessorViewTraits<AccessorTypes::SCALAR<T>>
    {
        using element_type = T;
        using value_type = T;
        using array_type = vsg::Array<value_type>;
        static constexpr std::size_t size = 1;
        template<typename TOther>
        using with_element_type = TOther;
    };

    template<typename T>
    struct AccessorViewTraits<AccessorTypes::VEC2<T>> : public VSGTraits<vsg::t_vec2, T>
    {
    };

    template<typename T>
    struct AccessorViewTraits<AccessorTypes::VEC3<T>> : public VSGTraits<vsg::t_vec3, T>
    {
    };

    template<typename T>
    struct AccessorViewTraits<AccessorTypes::VEC4<T>> : public VSGTraits<vsg::t_vec4, T>
    {
    };

    template<typename T>
    struct AccessorViewTraits<AccessorTypes::MAT3<T>> : public VSGTraits<vsg::t_mat3, T>
    {
    };

    template<typename T>
    struct AccessorViewTraits<AccessorTypes::MAT4<T>> : public VSGTraits<vsg::t_mat4, T>
    {
    };

    // No vsg t_mat2; will this work?
    template<typename T>
    struct AccessorViewTraits<AccessorTypes::MAT2<T>> : public VSGTraits<vsg::t_vec4, T>
    {
    };

    // Identify accessors that are valid as a draw index, so that much template expansion can be
    // avoided.

    template <typename T>
    struct is_index_type : std::false_type
    {
    };

    template<> struct is_index_type<AccessorTypes::SCALAR<uint8_t>> : std::true_type
    {
    };

    template<> struct is_index_type<AccessorTypes::SCALAR<uint16_t>> : std::true_type
    {
    };

    template<> struct is_index_type<AccessorTypes::SCALAR<uint32_t>> : std::true_type
    {
    };

    template <typename T>
    struct is_index_view_aux : std::false_type
    {
    };

    template <typename T>
    struct is_index_view_aux<AccessorView<T>> : is_index_type<T>
    {
    };

    template <typename T>
    struct is_index_view : is_index_view_aux<std::remove_reference_t<T>>
    {
    };

    // Convenience functions for accessing the elements of values in a vsg::Array, whether they are
    // scalar of vector

    template<typename T>
    typename T::value_type& atVSG(T& val, std::size_t i, std::enable_if_t<std::is_class_v<T>, bool> = true)
    {
        return val.data()[i];
    }

    template<typename T>
    T& atVSG(T& val, std::size_t, std::enable_if_t<!std::is_class_v<T>, bool> = true)
    {
        return val;
    }

    template<typename TA, typename TVSG = typename AccessorViewTraits<TA>::value_type, typename TArray = vsg::Array<TVSG>>
    vsg::ref_ptr<TArray> createArray(const AccessorView<TA>& accessorView)
    {
        static_assert(sizeof(TVSG) == sizeof(TA), "element sizes don't match");
        if (accessorView.status() != AccessorViewStatus::Valid)
        {
            throw std::runtime_error("invalid accessor view");
        }
        auto result = TArray::create(accessorView.size());
        for (int i = 0; i < accessorView.size(); ++i)
        {
            for (size_t j = 0; j < AccessorViewTraits<TA>::size; j++)
            {
                atVSG((*result)[i], j) = accessorView[i].value[j];
            }
        }
        return result;
    }

    template<typename TA, typename TI, typename TVSG = typename AccessorViewTraits<TA>::value_type,
             typename TArray = vsg::Array<TVSG>>
    vsg::ref_ptr<TArray> createArray(const AccessorView<TA>& accessorView,
                                     const AccessorView<TI>& indicesView)
    {
        static_assert(sizeof(TVSG) == sizeof(TA), "element sizes don't match");
        if (accessorView.status() != AccessorViewStatus::Valid)
        {
            throw std::runtime_error("invalid accessor view");
        }
        auto result = TArray::create(indicesView.size());
        for (int64_t i = 0; i < indicesView.size(); ++i)
        {
            for (size_t j = 0; j < AccessorViewTraits<TA>::size; ++j)
            {
                atVSG((*result)[i], j) = accessorView[indicesView[i].value[0]].value[j];
            }
        }
        return result;
    }

    // Doesn't support arrays of scalars
    template<typename F, typename TA,
             typename TDest = std::invoke_result_t<F, typename AccessorViewTraits<TA>::element_type>,
             typename TVSG = typename AccessorViewTraits<TA>::template with_element_type<TDest>,
             typename TArray = vsg::Array<TVSG>>
    vsg::ref_ptr<TArray> createArrayAndTransform(const AccessorView<TA>& accessorView, F&& f)
    {
        if (accessorView.status() != AccessorViewStatus::Valid)
        {
            throw std::runtime_error("invalid accessor view");
        }
        auto result = TArray::create(accessorView.size());
        for (int i = 0; i < accessorView.size(); ++i)
        {
            for (size_t j = 0; j < AccessorViewTraits<TA>::size; j++)
            {
                atVSG((*result)[i], j) = f(accessorView[i].value[j]);
            }
        }
        return result;
    }

    template<typename F, typename TA, typename TI,
             typename TDest = std::invoke_result_t<F, typename AccessorViewTraits<TA>::element_type>,
             typename TVSG = typename AccessorViewTraits<TA>::template with_element_type<TDest>,
             typename TArray = vsg::Array<TVSG>>
    vsg::ref_ptr<TArray> createArrayAndTransform(const AccessorView<TA>& accessorView, const AccessorView<TI>& indicesView,
                                                 const F& f)
    {
        // static_assert(sizeof(TVSG) == sizeof(TA), "element sizes don't match");
        if (accessorView.status() != AccessorViewStatus::Valid)
        {
            throw std::runtime_error("invalid accessor view");
        }
        auto result = TArray::create(accessorView.size());
        for (int i = 0; i < accessorView.size(); ++i)
        {
            for (size_t j = 0; j < AccessorViewTraits<TA>::size; j++)
            {
                atVSG((*result)[i], j) = f(accessorView[indicesView[i].value[0]].value[j]);
            }
        }
        return result;
    }

    template<typename S, typename D>
    D normalize(S val)
    {
        return val * (1.0f / std::numeric_limits<S>::max());
    }

    template<typename TV, typename TA>
    vsg::ref_ptr<vsg::Data> createNormalized(const AccessorView<TA>& accessorView)
    {
        return createArrayAndTransform(accessorView,
                                       normalize<typename AccessorViewTraits<TA>::element_type, TV>);

    }

    template<typename TV, typename TA, typename TI>
    vsg::ref_ptr<vsg::Data> createNormalized(const AccessorView<TA>& accessorView, const AccessorView<TI>& indicesView)
    {
        return createArrayAndTransform(accessorView, indicesView,
                                       normalize<typename AccessorViewTraits<TA>::element_type, TV>);

    }

    template<typename T, typename TA>
        vsg::ref_ptr<vsg::Array<T>> createArrayExplicit(const AccessorView<TA>& accessorView)
    {
        return createArray<TA, T>(accessorView);
    }

    // XXX Should probably write explicit functions for the valid index array types
    struct IndexVisitor
    {
        vsg::ref_ptr<vsg::Data> operator()(AccessorView<std::nullptr_t>&&) { return {}; }
        vsg::ref_ptr<vsg::Data> operator()(AccessorView<AccessorTypes::SCALAR<uint8_t>>&& view)
        {
            return createArrayAndTransform(view,
                                           [](uint8_t arg)
                                           {
                                               return static_cast<uint16_t>(arg);
                                           });
        }

        template<typename T>
        vsg::ref_ptr<vsg::Data> operator()(AccessorView<AccessorTypes::SCALAR<T>>&& view)
        {
            return createArrayExplicit<T>(view);
        }

        template<typename T>
        vsg::ref_ptr<vsg::Data> operator()(AccessorView<T>&&)
        {
            vsg::warn("Invalid array index type: ");
            return {};
        }
    };

    bool gltfToVk(int32_t mode, VkPrimitiveTopology& topology)
    {
        bool valid = true;
        switch (mode)
        {
        case MeshPrimitive::Mode::POINTS:
            topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case MeshPrimitive::Mode::LINES:
            topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case MeshPrimitive::Mode::LINE_STRIP:
            topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case MeshPrimitive::Mode::TRIANGLES:
            topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case MeshPrimitive::Mode::TRIANGLE_STRIP:
            topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case MeshPrimitive::Mode::TRIANGLE_FAN:
            topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            break;
        case MeshPrimitive::Mode::LINE_LOOP:
        default:
            valid = false;
        }
        return valid;
    }

    std::map<int32_t, int32_t>
    findTextureCoordAccessors(const std::string& prefix,
                              const std::unordered_map<std::string, int32_t>& attributes)
    {
        std::map<int32_t, int32_t> result;
        for (auto itr = attributes.begin(); itr != attributes.end(); ++itr)
        {
            const auto& name = itr->first;
            auto texCoords = getUintSuffix(prefix, name);
            if (texCoords)
            {
                result[texCoords.value()] = itr->second;
            }
        }
        return result;
    }

    template <typename R, typename F>
    R invokeWithAccessorViews(const Model* model, F&& f, const Accessor* accessor1, const Accessor* accessor2 = nullptr)
    {
        R result;
        if (accessor2)
        {
            createAccessorView(*model, *accessor1,
                               [model, &f, &result, accessor2](auto&& accessorView1)
                               {
                                   createAccessorView(*model, *accessor2,
                                                      [&f, &accessorView1, &result](auto&& accessorView2)
                                                      {
                                                          if constexpr(
                                                              is_index_view<decltype(accessorView2)>::value)
                                                          {
                                                              result = f(accessorView1, accessorView2);
                                                          }
                                                      });
                               });
        }
        else
        {
            createAccessorView(*model, *accessor1,
                               [&f, &result](auto&& accessorView1)
                               {
                                   result = f(accessorView1, AccessorView<AccessorTypes::SCALAR<uint16_t>>());
                               });
        }
        return result;
    }


    template<typename T, typename TI>
    vsg::ref_ptr<vsg::Data> colorProcessor(const AccessorView<AccessorTypes::VEC3<T>>& accessorView,
                                           const AccessorView<TI>& indexView)
    {
        vsg::ref_ptr<vsg::Data> result;
        if (std::is_same<T, float>::value)
        {
            if (indexView.status() == AccessorViewStatus::Valid)
            {
                result = createArray(accessorView, indexView);
            }
            else
            {
                result = createArray(accessorView);
            }
        }
        else
        {
            if (indexView.status() == AccessorViewStatus::Valid)
            {
                result = createNormalized<float>(accessorView, indexView);
            }
            else
            {
                result = createNormalized<float>(accessorView);
            }
        }

        result->properties.format = VK_FORMAT_R32G32B32_SFLOAT;
        return result;
    }

    template<typename T, typename TI>
    vsg::ref_ptr<vsg::Data> colorProcessor(const AccessorView<AccessorTypes::VEC4<T>>& accessorView,
                                           const AccessorView<TI>& indexView)
    {
        vsg::ref_ptr<vsg::Data> result;
        if (std::is_same<T, float>::value)
        {
            if (indexView.status() == AccessorViewStatus::Valid)
            {
                result = createArray(accessorView, indexView);
            }
            else
            {
                result = createArray(accessorView);
            }
        }
        else
        {
            if (indexView.status() == AccessorViewStatus::Valid)
            {
                result = createNormalized<float>(accessorView, indexView);
            }
            else
            {
                result = createNormalized<float>(accessorView);
            }
        }

        result->properties.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        return result;
    }

    template<typename T, typename TI>
    vsg::ref_ptr<vsg::Data> colorProcessor(const AccessorView<T>&, const AccessorView<TI>&) { return {}; } // invalidView


    vsg::ref_ptr<vsg::Data> doColors(const Model* model,
                                     const Accessor* dataAccessor, const Accessor* indexAccessor)
    {
        return invokeWithAccessorViews<vsg::ref_ptr<vsg::Data>>(model,
                                                                [](auto&& accessorView, auto&&indicesview)
                                                                {
                                                                    return colorProcessor(accessorView, indicesview);
                                                                },
                                                                dataAccessor, indexAccessor);
    }

    template<typename T, typename TI>
    vsg::ref_ptr<vsg::Data> texProcessor(const AccessorView<AccessorTypes::VEC2<T>>& accessorView,
                                         const AccessorView<TI>& indexView)
    {
        vsg::ref_ptr<vsg::Data> result;
        if (std::is_same<T, float>::value)
        {
            if (indexView.status() == AccessorViewStatus::Valid)
            {
                result = createArray(accessorView, indexView);
            }
            else
            {
                result = createArray(accessorView);
            }
        }
        else
        {
            if (indexView.status() == AccessorViewStatus::Valid)
            {
                result = createNormalized<float>(accessorView, indexView);
            }
            else
            {
                result = createNormalized<float>(accessorView);
            }
        }

        result->properties.format = VK_FORMAT_R32G32_SFLOAT;
        return result;
    }

    template<typename T, typename TI>
    vsg::ref_ptr<vsg::Data> texProcessor(const AccessorView<T>&, const AccessorView<TI>&) { return {}; } // invalidView
}


    vsg::ref_ptr<vsg::Data> doTextures(const Model* model,
                                       const Accessor* dataAccessor, const Accessor* indexAccessor)
    {
        return invokeWithAccessorViews<vsg::ref_ptr<vsg::Data>>(model,
                                                                [](const auto& accessorView, const auto& indicesview)
                                                                {
                                                                    return texProcessor(accessorView, indicesview);
                                                                },
                                                                dataAccessor, indexAccessor);
    }



template <typename A, typename I>
vsg::ref_ptr<vsg::Data> createData(Model* model, const A* dataAccessor, const I* indicesAccessor = nullptr)
{
    if (indicesAccessor)
    {
        return invokeWithAccessorViews<vsg::ref_ptr<vsg::Data>>(
            model,
            [](auto&& dataView, auto&&indicesView)
            {
                return vsg::ref_ptr<vsg::Data>(createArray(dataView, indicesView));
            },
            dataAccessor, indicesAccessor);
    }
    else
    {
        return  createAccessorView(*model, *dataAccessor,
                                   [](auto&& accessorView)
                                   {
                                       return vsg::ref_ptr<vsg::Data>(createArray(accessorView));
                                   });
    }
}

// I naively wrote the below comment:
// 
// Lots of hair for this in cesium-unreal. The main issues there seem to be 1) the need to recopy
// indices into a single format in Unreal and 2) support for duplicating vertices for generating
// normals and tangents. 1) isn't much of a problem (though uint8 is only supported with an
// extension), and 2) will be dealt with later. If we need to duplicate vertices, we will do that on
// the VSG data structures.
//
// 500 lines of code later, we have plenty of hair for duplicating vertex data,
// generating normals, recopying into the right data format... Most of our hair
// is in template functions, but there is a kind of conservation of hair going
// on here.

namespace
{
    // Helper function for getting an attribute accessor by name.
    const Accessor* getAccessor(const Model* model, const MeshPrimitive* primitive, std::string name)
    {
        auto itr = primitive->attributes.find(name);
        if (itr != primitive->attributes.end())
        {
            int accessorID = itr->second;
            return  Model::getSafe(&model->accessors, accessorID);
        }
        return nullptr;
    }
}

// Hack to construct a name for error message purposes, etc.

std::string ModelBuilder::makeName(const CesiumGltf::Mesh *mesh,
                                   const CesiumGltf::MeshPrimitive *primitive)
{
    std::string name = _name;
    std::ptrdiff_t meshNum = mesh - &_model->meshes[0];
    std::ptrdiff_t primNum = primitive - &mesh->primitives[0];
    if (meshNum >= 0 && static_cast<size_t>(meshNum) < _model->meshes.size())
    {
        name += " mesh " + std::to_string(meshNum);
    }
    if (primNum >= 0 && static_cast<size_t>(primNum) < mesh->primitives.size())
    {
        name += " primitive " + std::to_string(primNum);
    }
    return name;
}

vsg::ref_ptr<vsg::Node>
ModelBuilder::loadPrimitive(const CesiumGltf::MeshPrimitive* primitive,
                            const CesiumGltf::Mesh* mesh)
{
    const Accessor* pPositionAccessor = getAccessor(_model, primitive, "POSITION");
    if (!pPositionAccessor)
    {
        // Position accessor does not exist, so ignore this primitive.
        return {};
    }

    std::string name = makeName(mesh, primitive);
    VkPrimitiveTopology topology;
    if (!gltfToVk(primitive->mode, topology))
    {
        vsg::warn(name, ": Can't map glTF mode ", primitive->mode, " to Vulkan topology");
        return {};
    }
    int materialID = primitive->material;
    auto convertedMaterial = loadMaterial(materialID, topology);
    auto mat = convertedMaterial->descriptorConfig;
    auto config = MultisetPipelineConfigurator::create(mat->shaderSet);
    config->defines() = mat->defines;
    config->inputAssemblyState->topology = topology;
    if (topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
    {
        config->defines().insert("VSGCS_SIZE_TO_ERROR");
    }
    bool generateTangents = convertedMaterial->texInfo.count("normalMap") != 0
        && primitive->attributes.count("TANGENT") == 0;
    const Accessor* indicesAccessor = Model::getSafe(&_model->accessors, primitive->indices);
    const Accessor* normalAccessor = getAccessor(_model, primitive, "NORMAL");
    bool hasNormals = normalAccessor != nullptr;
    // The indices will be used to expand the attribute arrays.
    const Accessor* expansionIndices = ((!hasNormals || generateTangents) && indicesAccessor
                                        ? &_model->accessors[primitive->indices] : nullptr);
    vsg::DataList vertexArrays;
    auto positions = createData(_model, pPositionAccessor, expansionIndices);
    config->assignArray(vertexArrays, "vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, positions);
    if (normalAccessor)
    {
        config->assignArray(vertexArrays, "vsg_Normal", VK_VERTEX_INPUT_RATE_VERTEX,
                            ref_ptr_cast<vsg::vec3Array>(createData(_model, normalAccessor, expansionIndices)));
    }
    else if (config->inputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
    {
        config->defines().insert("VSGCS_BILLBOARD_NORMAL");
        // Won't be used unless we have a bizzare case of points and displacement map
        auto normal = vsg::vec3Value::create(vsg::vec3(0.0f, 1.0f, 0.0f));
        config->assignArray(vertexArrays, "vsg_Normal", VK_VERTEX_INPUT_RATE_INSTANCE, normal);
    }
    else
    {
        auto posArray = ref_ptr_cast<vsg::vec3Array>(positions);
        auto normals = vsg::vec3Array::create(posArray->size());
        generateNormals(posArray, normals, config->inputAssemblyState->topology);
        config->shaderHints->defines.insert("VSGCS_FLAT_SHADING");
        config->assignArray(vertexArrays, "vsg_Normal", VK_VERTEX_INPUT_RATE_VERTEX, normals);
    }

    // XXX
    // The VSG PBR shader doesn't use vertex tangent data, so we will skip the Cesium Unreal hair
    // for loading tangent data or generating it if necessary.
    //
    // We will probs change the shader to use tangent data.

    // XXX water mask

    // Bounding volumes aren't stored in most nodes in VSG and are computed when needed. Should we
    // store the the position min / max, or just not bother?

    const Accessor* colorAccessor = getAccessor(_model, primitive, "COLOR_0");
    vsg::ref_ptr<vsg::Data> colorData;
    if (colorAccessor
        && (colorData = doColors(_model, colorAccessor, expansionIndices)).valid())
    {
        config->assignArray(vertexArrays, "vsg_Color", VK_VERTEX_INPUT_RATE_VERTEX, colorData);
    }
    else
    {
        auto color = vsg::vec4Value::create(vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        config->assignArray(vertexArrays, "vsg_Color", VK_VERTEX_INPUT_RATE_INSTANCE, color);
    }

    // Textures...
    const auto& assignTexCoord = [&](std::string texPrefix, int baseLocation)
    {
        std::map<int32_t, int32_t> texAccessors = findTextureCoordAccessors(texPrefix, primitive->attributes);
        for (int i = 0; i < 2; ++i)
        {
            std::string arrayName = "vsg_TexCoord" + std::to_string(i + baseLocation);
            vsg::ref_ptr<vsg::Data> texdata;
            auto texcoordItr = texAccessors.find(i);
            if (texcoordItr != texAccessors.end())
            {
                const Accessor* texAccessor = Model::getSafe(&_model->accessors, texcoordItr->second);
                if (texAccessor)
                {
                    texdata = doTextures(_model, texAccessor, expansionIndices);
                }
            }
            if (texdata.valid())
            {
                config->assignArray(vertexArrays, arrayName, VK_VERTEX_INPUT_RATE_VERTEX, texdata);
            }
            else
            {
                auto texcoord = vsg::vec2Value::create(vsg::vec2(0.0f, 0.0f));
                config->assignArray(vertexArrays, arrayName, VK_VERTEX_INPUT_RATE_INSTANCE, texcoord);
            }
        }
    };
    assignTexCoord("TEXCOORD_", 0);
    assignTexCoord("_CESIUMOVERLAY_", 2);
    vsg::ref_ptr<vsg::Command> drawCommand;
    if (indicesAccessor && !expansionIndices)
    {
        vsg::ref_ptr<vsg::Data> indices = createAccessorView(*_model, *indicesAccessor, IndexVisitor());
        auto vid = vsg::VertexIndexDraw::create();
        vid->assignArrays(vertexArrays);
        vid->assignIndices(indices);
        vid->indexCount = static_cast<uint32_t>(indices->valueCount());
        vid->instanceCount = 1;
        drawCommand = vid;
    }
    else
    {
        auto vd = vsg::VertexDraw::create();
        vd->assignArrays(vertexArrays);
        vd->vertexCount = static_cast<uint32_t>(positions->valueCount());
        vd->instanceCount = 1;
        drawCommand = vd;
    }
    drawCommand->setValue("name", name);
    if (mat->blending)
    {
        // figure out what this means someday...
        // These are parameters for blending into the first color attachment in the render
        // pass. While this set of parameters implements bog-standard alpha blending, should they be
        // buried at this low level?
        //
        // Note that this will work for any "compatible" render pass too.
        config->colorBlendState->attachments = vsg::ColorBlendState::ColorBlendAttachments{
            {true, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
             VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_SUBTRACT,
             VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}};
        _builder->_sharedObjects->share(config->colorBlendState);
    }
    if (mat->two_sided)
    {
        config->rasterizationState->cullMode = VK_CULL_MODE_NONE;
    }
    if (mat->descriptorSet)
    {
        config->descriptorSetLayout = mat->descriptorSet->setLayout;
        config->descriptorBindings = mat->descriptorBindings;
    }
    config->init();
    _builder->_sharedObjects->share(config->bindGraphicsPipeline);

    auto stateGroup = vsg::StateGroup::create();
    stateGroup->add(config->bindGraphicsPipeline);

    if (mat->descriptorSet)
    {
        auto bindDescriptorSet
            = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, config->layout, pbr::PRIMITIVE_DESCRIPTOR_SET,
                                             mat->descriptorSet);
        stateGroup->add(bindDescriptorSet);
    }

    auto bindViewDescriptorSets = vsg::BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, config->layout, pbr::VIEW_DESCRIPTOR_SET);
    stateGroup->add(bindViewDescriptorSets);


    // assign any custom ArrayState that may be required.
    stateGroup->prototypeArrayState = config->shaderSet->getSuitableArrayState(config->shaderHints->defines);

    stateGroup->addChild(drawCommand);

    vsg::ComputeBounds computeBounds;
    drawCommand->accept(computeBounds);
    vsg::dvec3 center = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
    double radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.5;

    if (mat->blending)
    {
        auto depthSorted = vsg::DepthSorted::create();
        depthSorted->binNumber = 10;
        depthSorted->bound.set(center[0], center[1], center[2], radius);
        depthSorted->child = stateGroup;

        return depthSorted;
    }
    else
    {
        auto cullNode = vsg::CullNode::create(vsg::dsphere(center[0], center[1], center[2], radius),
                                              stateGroup);
        return cullNode;
    }
}

vsg::ref_ptr<ModelBuilder::ConvertedMaterial>
ModelBuilder::loadMaterial(const CesiumGltf::Material* material, VkPrimitiveTopology topology)
{
    auto convertedMat = ConvertedMaterial::create();
    convertedMat->descriptorConfig = DescriptorSetConfigurator::create();
    // XXX Cesium Unreal always enables two-sided, but it should come from the material...
    convertedMat->descriptorConfig->two_sided = true;
    convertedMat->descriptorConfig->defines.insert("VSG_TWO_SIDED_LIGHTING");
    if (_options.renderOverlays)
    {
        convertedMat->descriptorConfig->defines.insert("VSGCS_OVERLAY_MAPS");
    }
    vsg::PbrMaterial pbr;

    if (material->alphaMode == CesiumGltf::Material::AlphaMode::BLEND)
    {
        convertedMat->descriptorConfig->blending = true;
        pbr.alphaMaskCutoff = 0.0f;
    }
    convertedMat->descriptorConfig->shaderSet = _builder->getOrCreatePbrShaderSet(topology);
    if (material->pbrMetallicRoughness)
    {
        auto const& cesiumPbr = material->pbrMetallicRoughness.value();
        for (int i = 0; i < 3; ++i)
        {
            pbr.baseColorFactor[i] = static_cast<float>(cesiumPbr.baseColorFactor[i]);
        }
        if (cesiumPbr.baseColorFactor.size() > 3)
        {
            pbr.baseColorFactor[3] = static_cast<float>(cesiumPbr.baseColorFactor[3]);
        }
        if (cesiumPbr.metallicFactor >= 0.0)
        {
            pbr.metallicFactor = static_cast<float>(cesiumPbr.metallicFactor);
        }
        if (cesiumPbr.roughnessFactor >= 0.0)
        {
            pbr.roughnessFactor = static_cast<float>(cesiumPbr.roughnessFactor);
        }
        loadMaterialTexture(convertedMat, "diffuseMap", cesiumPbr.baseColorTexture, true);
        loadMaterialTexture(convertedMat, "mrMap", cesiumPbr.metallicRoughnessTexture, false);
    }
    loadMaterialTexture(convertedMat, "normalMap", material->normalTexture, false);
    loadMaterialTexture(convertedMat, "aoMap", material->occlusionTexture, false);
    loadMaterialTexture(convertedMat, "emissiveMap", material->emissiveTexture, true);
    convertedMat->descriptorConfig->assignUniform("material", vsg::PbrMaterialValue::create(pbr));
    auto descriptorSetLayout
        = vsg::DescriptorSetLayout::create(convertedMat->descriptorConfig->descriptorBindings);
    convertedMat->descriptorConfig->descriptorSet
        = vsg::DescriptorSet::create(descriptorSetLayout, convertedMat->descriptorConfig->descriptors);
    return convertedMat;
}

vsg::ref_ptr<ModelBuilder::ConvertedMaterial>
ModelBuilder::loadMaterial(int i, VkPrimitiveTopology topology)
{
    int topoIndex = topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST ? 1 : 0;

    if (i < 0 || static_cast<unsigned>(i) >= _model->materials.size())
    {
        if (!_defaultMaterial[topoIndex])
        {
            _defaultMaterial[topoIndex] = ConvertedMaterial::create();
            _defaultMaterial[topoIndex]->descriptorConfig = DescriptorSetConfigurator::create();
            _defaultMaterial[topoIndex]->descriptorConfig->shaderSet = _builder->getOrCreatePbrShaderSet(topology);
            vsg::PbrMaterial pbr;
            _defaultMaterial[topoIndex]->descriptorConfig->assignUniform("material",
                                                                         vsg::PbrMaterialValue::create(pbr));
        }
        return _defaultMaterial[topoIndex];
    }
    if (!_convertedMaterials[i][topoIndex])
    {
        _convertedMaterials[i][topoIndex] = loadMaterial(&_model->materials[i], topology);
    }
    return _convertedMaterials[i][topoIndex];
}

vsg::ref_ptr<vsg::Group>
ModelBuilder::operator()()
{
    vsg::ref_ptr<vsg::Group> resultNode = vsg::Group::create();

    if (_model->scene >= 0 && static_cast<unsigned>(_model->scene) < _model->scenes.size())
    {
        // Show the default scene
        const Scene& defaultScene = _model->scenes[_model->scene];
        for (int nodeId : defaultScene.nodes)
        {
            resultNode->addChild(loadNode(&_model->nodes[nodeId]));
        }
    }
    else if (_model->scenes.size() > 0)
    {
        // There's no default, so show the first scene
        const Scene& defaultScene = _model->scenes[0];
        for (int nodeId : defaultScene.nodes)
        {
            resultNode->addChild(loadNode(&_model->nodes[nodeId]));
        }
    }
    else if (_model->nodes.size() > 0)
    {
        // No scenes at all, use the first node as the root node.
        resultNode = loadNode(&_model->nodes[0]);
    }
    else if (_model->meshes.size() > 0)
    {
        // No nodes either, show all the meshes.
        for (const Mesh& mesh : _model->meshes)
        {
            resultNode->addChild(loadMesh(&mesh));
        }
    }
    return resultNode;
}

// Hold Raster data that we can attach to the VSG tile in order to easily find
// it later.
//

struct RasterData
{
    std::string name;
    vsg::ref_ptr<vsg::ImageInfo> rasterImage;
    pbr::OverlayParams overlayParams;
};

struct Rasters : public vsg::Inherit<vsg::Object, Rasters>
{
    Rasters(size_t numOverlays)
        : overlayRasters(numOverlays)
    {
    }
    std::vector<RasterData> overlayRasters;
};

vsg::ref_ptr<vsg::StateCommand> makeTileStateCommand(CesiumGltfBuilder& builder, const Rasters& rasters,
                                                        const Cesium3DTilesSelection::Tile& tile)
{
    vsg::ImageInfoList rasterImages(rasters.overlayRasters.size());
    // The topology doesn't matter because the pipeline layouts of shader versions are compatible.
    vsg::ref_ptr<DescriptorSetConfigurator> descriptorBuilder
        = DescriptorSetConfigurator::create(pbr::TILE_DESCRIPTOR_SET,
                                            builder.getOrCreatePbrShaderSet(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST));
    std::vector<pbr::OverlayParams> overlayParams(rasters.overlayRasters.size());
    for (size_t i = 0; i < rasters.overlayRasters.size(); ++i)
    {
        const auto& rasterData = rasters.overlayRasters[i];
        rasterImages[i] = rasterData.rasterImage.valid()
            ? rasterData.rasterImage : builder.getDefaultTexture();
        overlayParams[i] = rasterData.overlayParams;
    }
    descriptorBuilder->assignTexture("overlayTextures", rasterImages);
    auto ubo = pbr::makeTileData(tile.getGeometricError(), std::min(builder.getFeatures().pointSizeRange[1], 4.0f),
                                 overlayParams);
    descriptorBuilder->assignUniform("tileParams", ubo);
    descriptorBuilder->init();
    auto bindDescriptorSet
            = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                             builder.getOverlayPipelineLayout(), pbr::TILE_DESCRIPTOR_SET,
                                             descriptorBuilder->descriptorSet);
    return bindDescriptorSet;
 }


vsg::ref_ptr<vsg::Group>
CesiumGltfBuilder::load(CesiumGltf::Model* model, const CreateModelOptions& options)
{
    ModelBuilder builder(this, model, options);
    return builder();
}

vsg::ref_ptr<vsg::Node> CesiumGltfBuilder::loadTile(Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
                                                    const glm::dmat4 &transform,
                                                    const CreateModelOptions& modelOptions)
{
    CesiumGltf::Model* pModel = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);
    if (!pModel)
    {
        return {};
    }
    Model& model = *pModel;
    glm::dmat4x4 rootTransform = transform;

    rootTransform = Cesium3DTilesSelection::GltfUtilities::applyRtcCenter(model, rootTransform);
    rootTransform = Cesium3DTilesSelection::GltfUtilities::applyGltfUpAxisTransform(model, rootTransform);
    auto transformNode = vsg::MatrixTransform::create(glm2vsg(rootTransform));
    auto modelNode = load(pModel, modelOptions);
    auto tileStateGroup = vsg::StateGroup::create();
    auto bindViewDescriptorSets = vsg::BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                                      _overlayPipelineLayout, 0);
    // Make uniforms (tile and raster parameters) and default textures for the tile.

    auto rasters = Rasters::create(pbr::maxOverlays);
    transformNode->setObject("vsgCs_rasterData", rasters);

    tileStateGroup->add(bindViewDescriptorSets);
    tileStateGroup->addChild(modelNode);
    transformNode->addChild(tileStateGroup);
    return transformNode;
}

vsg::ref_ptr<vsg::Object> CesiumGltfBuilder::attachTileData(Cesium3DTilesSelection::Tile& tile, vsg::ref_ptr<vsg::Node> node)
{
    auto rasters = Rasters::create(pbr::maxOverlays);
    auto transformNode = ref_ptr_cast<vsg::MatrixTransform>(node);
    auto tileStateGroup = ref_ptr_cast<vsg::StateGroup>(transformNode->children[0]);
    auto tileStateCommand = makeTileStateCommand(*this, *rasters, tile);
    tileStateGroup->add(tileStateCommand);
    return tileStateCommand;
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
            return sRGB ? VK_FORMAT_UNDEFINED : VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
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

        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            return BlockSize(4, 4);
        default:
            return BlockSize(1, 1);
        }
    }

    vsg::ref_ptr<vsg::Data>
    makeArray(uint32_t width, uint32_t height, void* data, vsg::Data::Properties in_properties)
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
                                             reinterpret_cast<vsg::ubvec2Array2D::value_type*>(data),
                                             in_properties);
            
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
            return vsg::ubvec4Array2D::create(width, height,
                                             reinterpret_cast<vsg::ubvec4Array2D::value_type*>(data),
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
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
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
            = new (vsg::allocate(sizeof(uint8_t) * destSize, vsg::ALLOCATOR_AFFINITY_DATA)) uint8_t[destSize];
        uint8_t* pDest = destData;
        auto srcItr = image.pixelData.begin();
        while (srcItr != image.pixelData.end())
        {
            for (int i = 0; i < 3; ++i)
            {
                *pDest++ = std::to_integer<uint8_t>(*srcItr++);
            }
            *pDest++ = 1;
        }
        return destData;
    }
}

namespace vsgCs
{

vsg::ref_ptr<vsg::Data> loadImage(CesiumGltf::ImageCesium& image, bool useMipMaps, bool sRGB)
{
    if (image.pixelData.empty() || image.width == 0 || image.height == 0)
    {
        return {};
    }
    if (useMipMaps && image.mipPositions.empty())
    {
        std::optional<std::string> errorMsg =
            CesiumGltfReader::GltfReader::generateMipMaps(image);
        if (errorMsg)
        {
            vsg::warn(errorMsg.value());
        }
    }
    VkFormat pixelFormat = cesiumToVk(image, sRGB);
    if (pixelFormat == VK_FORMAT_UNDEFINED)
    {
        return {};
    }
    vsg::Data::Properties props;
    props.format = pixelFormat;
    props.maxNumMipmaps = static_cast<uint8_t>(image.mipPositions.size());
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
            = new (vsg::allocate(sizeof(uint8_t) * image.pixelData.size(), vsg::ALLOCATOR_AFFINITY_DATA))
            uint8_t[image.pixelData.size()];
        std::transform(image.pixelData.begin(), image.pixelData.end(), destData,
                       [](std::byte b)
                       {
                           return std::to_integer<uint8_t>(b);
                       });
        imageSource = destData;
    }
    // Assume that there is no advantage in sharing the texture data; might be very false!
    return makeArray(image.width, image.height, imageSource, props);
}
}

vsg::ref_ptr<vsg::Data> ModelBuilder::loadImage(int i, bool useMipMaps, bool sRGB)
{
    CesiumGltf::ImageCesium& image = _model->images[i].cesium;
    ImageData& imageData = _loadedImages[i];
    if ((imageData.image.valid() || imageData.imageWithMipmap.valid())
        && sRGB != imageData.sRGB)
    {
        vsg::warn(_name, ": image ", i, " used as linear and sRGB");
    }
    if (imageData.imageWithMipmap.valid())
    {
        return imageData.imageWithMipmap;
    }
    else if (!useMipMaps && imageData.image.valid())
    {
        return imageData.image;
    }
    auto data = vsgCs::loadImage(image, useMipMaps, sRGB);
    imageData.sRGB = sRGB;
    if (useMipMaps)
    {
        return imageData.imageWithMipmap = data;
    }
    else
    {
        return imageData.image = data;
    }
}

namespace vsgCs
{
    vsg::ref_ptr<vsg::Sampler> makeSampler(VkSamplerAddressMode addressX,
                                           VkSamplerAddressMode addressY,
                                           VkFilter minFilter,
                                           VkFilter maxFilter,
                                           int maxNumMipMaps)
    {
        auto result = vsg::Sampler::create();
        if (maxNumMipMaps > 1)
        {
            result->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
        else
        {
            result->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
        result->minFilter = minFilter;
        result->magFilter = maxFilter;
        result->addressModeU = addressX;
        result->addressModeV = addressY;
        result->addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        result->anisotropyEnable = VK_TRUE;
        result->maxAnisotropy = 16.0f;
        result->maxLod = static_cast<float>(maxNumMipMaps);
        return result;
    }
}

vsg::ref_ptr<vsg::ImageInfo> CesiumGltfBuilder::loadTexture(CesiumTextureSource&& imageSource,
                                                            VkSamplerAddressMode addressX,
                                                            VkSamplerAddressMode addressY,
                                                            VkFilter minFilter,
                                                            VkFilter maxFilter,
                                                            bool useMipMaps,
                                                            bool sRGB)
{
    CesiumGltf::ImageCesium* pImage =
        std::visit(GetImageFromSource{}, imageSource);
    return loadTexture(*pImage, addressX, addressY, minFilter, maxFilter, useMipMaps, sRGB);
}

vsg::ref_ptr<vsg::ImageInfo> CesiumGltfBuilder::loadTexture(CesiumGltf::ImageCesium& image,
                                                            VkSamplerAddressMode addressX,
                                                            VkSamplerAddressMode addressY,
                                                            VkFilter minFilter,
                                                            VkFilter maxFilter,
                                                            bool useMipMaps,
                                                            bool sRGB)
{
    auto data = loadImage(image, useMipMaps, sRGB);
    if (!data)
    {
        return {};
    }
    auto sampler = makeSampler(addressX, addressY, minFilter, maxFilter,
                                 data->properties.maxNumMipmaps);
    _sharedObjects->share(sampler);
    return vsg::ImageInfo::create(sampler, data);
}

vsg::ref_ptr<vsg::ImageInfo> ModelBuilder::loadTexture(const CesiumGltf::Texture& texture,
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
            static_cast<unsigned>(pKtxExtension->source) >= _model->images.size())
        {
            vsg::warn("KTX texture source index must be non-negative and less than ",
                      _model->images.size(),
                      " but is ",
                      pKtxExtension->source);
            return {};
        }
        source = pKtxExtension->source;
    }
    else if (pWebpExtension)
    {
        if (pWebpExtension->source < 0 ||
            static_cast<unsigned>(pWebpExtension->source) >= _model->images.size())
        {
            vsg::warn("WebP texture source index must be non-negative and less than ",
                      _model->images.size(),
                      " but is ",
                      pWebpExtension->source);
            return {};
        }
        source = pWebpExtension->source;
    }
    else
    {
        if (texture.source < 0 || static_cast<unsigned>(texture.source) >= _model->images.size())
        {
            vsg::warn("Texture source index must be non-negative and less than ",
                      _model->images.size(),
                      " but is ",
                      texture.source);
            return {};
        }
        source = texture.source;
    }

    const CesiumGltf::Sampler* pSampler =
        CesiumGltf::Model::getSafe(&_model->samplers, texture.sampler);
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
    auto data = loadImage(source, useMipMaps, sRGB);
    auto sampler = makeSampler(addressX, addressY, minFilter, magFilter,
                               data->properties.maxNumMipmaps);
    _builder->_sharedObjects->share(sampler);
    return vsg::ImageInfo::create(sampler, data);
}

ModifyRastersResult CesiumGltfBuilder::attachRaster(const Cesium3DTilesSelection::Tile& tile,
                                                    vsg::ref_ptr<vsg::Node> node,
                                                    int32_t overlayTextureCoordinateID,
                                                    const Cesium3DTilesSelection::RasterOverlayTile&,
                                                    void* pMainThreadRendererResources,
                                                    const glm::dvec2& translation,
                                                    const glm::dvec2& scale)
{
    ModifyRastersResult result;
    vsg::ref_ptr<vsg::MatrixTransform> matrixTransform = node.cast<vsg::MatrixTransform>();
    if (!matrixTransform)
        return {};                 // uhhhh
    vsg::ref_ptr<Rasters> rasters(matrixTransform->getObject<Rasters>("vsgCs_rasterData"));
    if (!rasters)
    {
        rasters = Rasters::create(pbr::maxOverlays);
        matrixTransform->setObject("vsgCs_rasterData", rasters);
    }

    vsg::ref_ptr<vsg::StateGroup> stateGroup = matrixTransform->children[0].cast<vsg::StateGroup>();
    if (!stateGroup)
        return {};
    RasterResources *resource = static_cast<RasterResources*>(pMainThreadRendererResources);
    auto raster = resource->raster;
    auto& rasterData = rasters->overlayRasters.at(resource->overlayOptions.layerNumber);
    rasterData.rasterImage = raster;
    rasterData.overlayParams.translation = glm2vsg(translation);
    rasterData.overlayParams.scale = glm2vsg(scale);
    rasterData.overlayParams.coordIndex = overlayTextureCoordinateID;
    rasterData.overlayParams.enabled = 1;
    rasterData.overlayParams.alpha = resource->overlayOptions.alpha;
    auto command = makeTileStateCommand(*this, *rasters, tile);
    // XXX Should check data or something in the state command instead of relying on the number of
    // commands in the group.
    auto& stateCommands = stateGroup->stateCommands;
    if (stateCommands.size() > 1)
    {
        result.deleteObjects.insert(result.deleteObjects.end(),
                                    stateCommands.begin() + 1, stateCommands.end());
        stateCommands.erase(stateCommands.begin() + 1, stateCommands.end());
    }
    stateCommands.push_back(command);
    result.compileObjects.push_back(command);
    return result;
}

vsg::ref_ptr<vsg::ImageInfo> CesiumGltfBuilder::makeDefaultTexture()
{
    vsg::ubvec4 pixel(255, 255, 255, 255);
    vsg::Data::Properties props;
    props.format = VK_FORMAT_R8G8B8A8_UNORM;
    props.maxNumMipmaps = 1;
    auto arrayData = makeArray(1, 1, &pixel, props);
    auto sampler = makeSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                               VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_NEAREST, VK_FILTER_NEAREST, 1);
    return vsg::ImageInfo::create(sampler, arrayData);
}

ModifyRastersResult
CesiumGltfBuilder::detachRaster(const Cesium3DTilesSelection::Tile& tile,
                                vsg::ref_ptr<vsg::Node> node,
                                int32_t,
                                const Cesium3DTilesSelection::RasterOverlayTile& rasterTile)
{
    ModifyRastersResult result;
    vsg::ref_ptr<vsg::MatrixTransform> matrixTransform = node.cast<vsg::MatrixTransform>();
    if (!matrixTransform)
        return {};                 // uhhhh
    vsg::ref_ptr<Rasters> rasters(matrixTransform->getObject<Rasters>("vsgCs_rasterData"));
    if (!rasters)
    {
        return {};
    }
    vsg::ref_ptr<vsg::StateGroup> stateGroup = matrixTransform->children[0].cast<vsg::StateGroup>();
    if (!stateGroup)
        return {};
    RasterResources *resource = static_cast<RasterResources*>(rasterTile.getRendererResources());
    auto& rasterData = rasters->overlayRasters.at(resource->overlayOptions.layerNumber);
    {
        rasterData.rasterImage = {}; // ref to rasterImage is still held by the old StateCommand
        rasterData.overlayParams.enabled = 0;
        auto newCommand = makeTileStateCommand(*this, *rasters, tile);
        vsg::ref_ptr<vsg::Command> oldCommand;
        // XXX Should check data or something in the state command instead of relying on the number of
        // commands in the group.
        auto& stateCommands = stateGroup->stateCommands;
        if (stateCommands.size() > 1)
        {
            oldCommand = *(stateCommands.begin() + 1);
            stateCommands.erase(stateCommands.begin() + 1);
        }
        stateCommands.push_back(newCommand);
        result.compileObjects.push_back(newCommand);
        result.deleteObjects.push_back(oldCommand);
    }
    return result;
}
