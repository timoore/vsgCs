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

#include "CppAllocator.h"

#include <cstdlib>

namespace vsgCs
{
    void* CppAllocator::allocate(std::size_t size, vsg::AllocatorAffinity allocatorAffinity)
    {
        std::size_t alignment = 8;
        if (allocatorAffinity == vsg::ALLOCATOR_AFFINITY_PHYSICS)
        {
            alignment = 16;
        }
        std::size_t roundedSize = size;
        if (auto mod = roundedSize % alignment; mod != 0)
        {
            roundedSize += alignment - mod;
        }
        return aligned_alloc(alignment, roundedSize);
    }

    bool CppAllocator::deallocate(void* ptr, std::size_t)
    {
        free(ptr);
        return true;
    }

    std::size_t CppAllocator::deleteEmptyMemoryBlocks()
    {
        return 0;
    }

    std::size_t CppAllocator::totalAvailableSize() const
    {
        return 1024 * 1024 * 1024;
    }

    std::size_t CppAllocator::totalReservedSize() const
    {
        return 1024 * 1024;
    }

    std::size_t CppAllocator::totalMemorySize() const
    {
        return 1024 * 1024 * 1024;
    }

    void CppAllocator::setBlockSize(vsg::AllocatorAffinity, std::size_t)
    {
    }

    void CppAllocator::report(std::ostream& out) const
    {
        out << "no info\n";
    }
}
