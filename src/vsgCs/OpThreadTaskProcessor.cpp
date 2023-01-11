#include "OpThreadTaskProcessor.h"

namespace vsgCs
{
    AsyncSystemWrapper& getAsyncSystemWrapper()
    {
        static AsyncSystemWrapper wrapper;
        return wrapper;
    }
}

using namespace vsgCs;

AsyncSystemWrapper::AsyncSystemWrapper()
    : taskProcessor(std::make_shared<OpThreadTaskProcessor>(4)),
      asyncSystem(taskProcessor)
{
}

void AsyncSystemWrapper::shutdown()
{
    asyncSystem.dispatchMainThreadTasks();
    taskProcessor->stop();
}

class TaskOperation : public vsg::Inherit<vsg::Operation, TaskOperation>
{
public:
    TaskOperation(std::function<void()> f)
        : _f(f)
    {
    }

    void run() override
    {
        _f();
    }
private:
    std::function<void()> _f;

};

OpThreadTaskProcessor::OpThreadTaskProcessor(uint32_t numThreads)
{
    _opthreads = vsg::OperationThreads::create(numThreads);
}

void OpThreadTaskProcessor::stop()
{
    _opthreads->stop();
}

OpThreadTaskProcessor::~OpThreadTaskProcessor()
{
    _opthreads->stop();
}

void OpThreadTaskProcessor::startTask(std::function<void()> f)
{
    _opthreads->add(TaskOperation::create(f));
}

