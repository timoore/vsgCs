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

#include "accessor_traits.h"

#include <vsg/io/Logger.h>

namespace vsgCs
{
    // Convenience functions for accessing the elements of values in a vsg::Array, whether they are
    // scalar or vector

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

    /**
     * @brief Create a vsg data array from a Cesium AccessorView.
     */
    template<typename TA, typename TVSG = typename AccessorViewTraits<TA>::value_type, typename TArray = vsg::Array<TVSG>>
    vsg::ref_ptr<TArray> createArray(const CesiumGltf::AccessorView<TA>& accessorView)
    {
        static_assert(sizeof(TVSG) == sizeof(TA), "element sizes don't match");
        if (accessorView.status() != CesiumGltf::AccessorViewStatus::Valid)
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

    /**
     * @brief Create a vsg data array from a Cesium AccessorView of data, using a second accessor
     * view of indices to copy elements from the data view.
     */
    template<typename TA, typename TI, typename TVSG = typename AccessorViewTraits<TA>::value_type,
             typename TArray = vsg::Array<TVSG>>
    vsg::ref_ptr<TArray> createArray(const CesiumGltf::AccessorView<TA>& accessorView,
                                     const CesiumGltf::AccessorView<TI>& indicesView)
    {
        static_assert(sizeof(TVSG) == sizeof(TA), "element sizes don't match");
        if (accessorView.status() != CesiumGltf::AccessorViewStatus::Valid)
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

    /**
     * @brief Create a vsg data array from a Cesium AccessorView, invoking a functional on each element.
     * Doesn't support arrays of scalars
     */
    template<typename F, typename TA,
             typename TDest = std::invoke_result_t<F, typename AccessorViewTraits<TA>::element_type>,
             typename TVSG = typename AccessorViewTraits<TA>::template with_element_type<TDest>,
             typename TArray = vsg::Array<TVSG>>
    vsg::ref_ptr<TArray> createArrayAndTransform(const CesiumGltf::AccessorView<TA>& accessorView, F&& f)
    {
        if (accessorView.status() != CesiumGltf::AccessorViewStatus::Valid)
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

    /**
     * @brief Create a vsg data array from a Cesium AccessorView of data, using a second accessor
     * view of indices to copy elements from the data view. A functional is invoked for each element copied.
     */

    template<typename F, typename TA, typename TI,
             typename TDest = std::invoke_result_t<F, typename AccessorViewTraits<TA>::element_type>,
             typename TVSG = typename AccessorViewTraits<TA>::template with_element_type<TDest>,
             typename TArray = vsg::Array<TVSG>>
    vsg::ref_ptr<TArray> createArrayAndTransform(const CesiumGltf::AccessorView<TA>& accessorView, const CesiumGltf::AccessorView<TI>& indicesView,
                                                 const F& f)
    {
        // static_assert(sizeof(TVSG) == sizeof(TA), "element sizes don't match");
        if (accessorView.status() != CesiumGltf::AccessorViewStatus::Valid)
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
        return static_cast<D>(val * (1.0 / std::numeric_limits<S>::max()));
    }

    template<typename TV, typename TA>
    vsg::ref_ptr<vsg::Data> createNormalized(const CesiumGltf::AccessorView<TA>& accessorView)
    {
        return createArrayAndTransform(accessorView,
                                       normalize<typename AccessorViewTraits<TA>::element_type, TV>);

    }

    template<typename TV, typename TA, typename TI>
    vsg::ref_ptr<vsg::Data> createNormalized(const CesiumGltf::AccessorView<TA>& accessorView, const CesiumGltf::AccessorView<TI>& indicesView)
    {
        return createArrayAndTransform(accessorView, indicesView,
                                       normalize<typename AccessorViewTraits<TA>::element_type, TV>);

    }

    template<typename T, typename TA>
        vsg::ref_ptr<vsg::Array<T>> createArrayExplicit(const CesiumGltf::AccessorView<TA>& accessorView)
    {
        return createArray<TA, T>(accessorView);
    }

    // XXX Should probably write explicit functions for the valid index array types
    struct IndexVisitor
    {
        vsg::ref_ptr<vsg::Data> operator()(CesiumGltf::AccessorView<std::nullptr_t>&&) { return {}; }
        vsg::ref_ptr<vsg::Data> operator()(CesiumGltf::AccessorView<CesiumGltf::AccessorTypes::SCALAR<uint8_t>>&& view)
        {
            return createArrayAndTransform(view,
                                           [](uint8_t arg)
                                           {
                                               return static_cast<uint16_t>(arg);
                                           });
        }

        template<typename T>
        vsg::ref_ptr<vsg::Data> operator()(CesiumGltf::AccessorView<CesiumGltf::AccessorTypes::SCALAR<T>>&& view)
        {
            return createArrayExplicit<T>(view);
        }

        template<typename T>
        vsg::ref_ptr<vsg::Data> operator()(CesiumGltf::AccessorView<T>&&)
        {
            vsg::warn("Invalid array index type: ");
            return {};
        }
    };

    /**
     * @brief Invoke a functional with AccessorView parameters for a data and optional indices.
     */
    template <typename R, typename F>
    R invokeWithAccessorViews(const CesiumGltf::Model* model, F&& f, const CesiumGltf::Accessor* accessor1, const CesiumGltf::Accessor* accessor2 = nullptr)
    {
        R result;
        if (accessor2)
        {
            createAccessorView(
                *model, *accessor1,
                [model, &f, &result, accessor2](auto&& accessorView1)
                {
                    createAccessorView(
                        *model, *accessor2,
                        [&f, &accessorView1, &result](auto&& accessorView2)
                        {
                            if constexpr(is_index_view<decltype(accessorView2)>::value)
                            {
                                result = f(accessorView1, accessorView2);
                            }
                        });
                });
        }
        else
        {
            createAccessorView(
                *model, *accessor1,
                [&f, &result](auto&& accessorView1)
                {
                    result = f(accessorView1, CesiumGltf::AccessorView<CesiumGltf::AccessorTypes::SCALAR<uint16_t>>());
                });
        }
        return result;
    }
}
