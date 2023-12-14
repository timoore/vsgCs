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

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#include <vsg/vk/vulkan.h>

#include "tracy/TracyVulkan.hpp"

#define VSGCS_ZONESCOPED ZoneScoped
#define VSGCS_ZONESCOPEDN(name) ZoneScopedN(name)
#define VSGCS_FRAMEMARK FrameMark

#else
#define VSGCS_ZONESCOPED
#define VSGCS_ZONESCOPEDN(name)
#define VSGCS_FRAMEMARK
#endif

#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>

#include <cstdint>

namespace vsgCs
{
    class TracyContextValue : public vsg::Inherit<vsg::Object, TracyContextValue>
    {
    public:
#ifdef TRACY_ENABLE
        tracy::VkCtx* ctx = nullptr;
#else
        void* ctx = nullptr;
#endif
        uint64_t tracingMask = 0;
        uint64_t gpuProfilingMask = 0;
    };
}
