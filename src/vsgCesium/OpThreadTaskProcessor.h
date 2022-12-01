#pragma once

#include "Export.h"
#include <CesiumAsync/ITaskProcessor.h>
#include <vsg/all.h>

namespace vsgCesium
{

    class VSGCESIUM_EXPORT OpThreadTaskProcessor : public CesiumAsync::ITaskProcessor
    {
    public:
        OpThreadTaskProcessor(uint32_t numThreads);
        ~OpThreadTaskProcessor();
        virtual void startTask(std::function<void()> f) override;
    private:
        vsg::ref_ptr<vsg::OperationThreads> _opthreads;
    };
}
