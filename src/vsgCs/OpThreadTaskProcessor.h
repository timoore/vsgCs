#pragma once

#include "Export.h"
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
