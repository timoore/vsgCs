/* <editor-fold desc="MIT License">

Copyright(c) 2025 Timothy Moore

based on code that is

Copyright (c) 2018 Robert Osfield

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

#include <vsg/core/Allocator.h>

#include <cstddef>
#include <memory>
#include <ostream>

// Drop-in replacement for VSG's custom allocator, using only
// aligned_alloc and free. This is useful for use with valgrind and
// other leak detection tools that track malloc() and free().

namespace vsgCs
{
    class CppAllocator : public vsg::Allocator
    {
    public:
        void* allocate(std::size_t size, vsg::AllocatorAffinity allocatorAffinity) override;
        bool deallocate(void* ptr, std::size_t size) override;
        size_t deleteEmptyMemoryBlocks() override;

        /// return the total available size of allocated MemoryBlocks
        size_t totalAvailableSize() const override;

        /// return the total reserved size of allocated MemoryBlocks
        size_t totalReservedSize() const override;

        /// return the total memory size of allocated MemoryBlocks
        size_t totalMemorySize() const override;

        void setBlockSize(vsg::AllocatorAffinity allocatorAffinity, size_t blockSize) override;
        void report(std::ostream& out) const override;
    };
}
