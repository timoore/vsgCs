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

#include <CesiumGltf/AccessorView.h>
#include <vsg/core/Array.h>
#include <vsg/maths/mat3.h>
#include <vsg/maths/mat4.h>

namespace vsgCs
{
    // Helpers for writing templates that take Cesium Accessors as arguments.

    template<typename T> struct AccessorViewTraits;

    template<template<typename> typename  t_vsg, typename TValue>
    struct VSGTraits
    {
        using element_type = typename t_vsg<TValue>::value_type;
        using value_type = t_vsg<TValue>;
        using array_type = vsg::Array<value_type>;
        static constexpr std::size_t size = t_vsg<TValue>().size();
        template<typename TOther>
        using with_element_type = t_vsg<TOther>;
    };

    template<typename T>
    struct AccessorViewTraits<CesiumGltf::AccessorTypes::SCALAR<T>>
    {
        using element_type = T;
        using value_type = T;
        using array_type = vsg::Array<value_type>;
        static constexpr std::size_t size = 1;
        template<typename TOther>
        using with_element_type = TOther;
    };

    template<typename T>
    struct AccessorViewTraits<CesiumGltf::AccessorTypes::VEC2<T>> : VSGTraits<vsg::t_vec2, T>
    {
    };

    template<typename T>
    struct AccessorViewTraits<CesiumGltf::AccessorTypes::VEC3<T>> : VSGTraits<vsg::t_vec3, T>
    {
    };

    template<typename T>
    struct AccessorViewTraits<CesiumGltf::AccessorTypes::VEC4<T>> : VSGTraits<vsg::t_vec4, T>
    {
    };

    template<typename T>
    struct AccessorViewTraits<CesiumGltf::AccessorTypes::MAT3<T>> : VSGTraits<vsg::t_mat3, T>
    {
    };

    template<typename T>
    struct AccessorViewTraits<CesiumGltf::AccessorTypes::MAT4<T>> : VSGTraits<vsg::t_mat4, T>
    {
    };

    // No vsg t_mat2; will this work?
    template<typename T>
    struct AccessorViewTraits<CesiumGltf::AccessorTypes::MAT2<T>> : VSGTraits<vsg::t_vec4, T>
    {
    };

    // Identify accessors that are valid as a draw index, so that much template expansion can be
    // avoided.

    template <typename T>
    struct is_index_type : std::false_type
    {
    };

    template<> struct is_index_type<CesiumGltf::AccessorTypes::SCALAR<uint8_t>> : std::true_type
    {
    };

    template<> struct is_index_type<CesiumGltf::AccessorTypes::SCALAR<uint16_t>> : std::true_type
    {
    };

    template<> struct is_index_type<CesiumGltf::AccessorTypes::SCALAR<uint32_t>> : std::true_type
    {
    };

    template <typename T>
    struct is_index_view_aux : std::false_type
    {
    };

    template <typename T>
    struct is_index_view_aux<CesiumGltf::AccessorView<T>> : is_index_type<T>
    {
    };

    template <typename T>
    struct is_index_view : is_index_view_aux<std::remove_reference_t<T>>
    {
    };
}
