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
#include <CesiumAsync/ITaskProcessor.h>
#include <CesiumAsync/AsyncSystem.h>
#include <vsg/all.h>

namespace vsgCs
{
    // Implement Cesium's ITaskprocessor interface, the basis for its AsyncSystem.
    class VSGCS_EXPORT OpThreadTaskProcessor : public CesiumAsync::ITaskProcessor
    {
    public:
        OpThreadTaskProcessor(uint32_t numThreads);
        ~OpThreadTaskProcessor();
        virtual void startTask(std::function<void()> f) override;
        virtual void stop();
    private:
        vsg::ref_ptr<vsg::OperationThreads> _opthreads;
    };

    // Wrapper class that allows shutting down the AsyncSystem when the program exits.

    class VSGCS_EXPORT AsyncSystemWrapper
    {
    public:
        AsyncSystemWrapper();
        CesiumAsync::AsyncSystem& getAsyncSystem() noexcept;
        void shutdown();
        std::shared_ptr<OpThreadTaskProcessor> taskProcessor;
        CesiumAsync::AsyncSystem asyncSystem;
    };

    AsyncSystemWrapper& VSGCS_EXPORT getAsyncSystemWrapper();
    inline CesiumAsync::AsyncSystem& getAsyncSystem() noexcept
    {
        return getAsyncSystemWrapper().asyncSystem;
    }
}
