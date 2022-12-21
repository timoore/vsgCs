#include "OpThreadTaskProcessor.h"

using namespace vsgCs;

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

OpThreadTaskProcessor::~OpThreadTaskProcessor()
{
    _opthreads->stop();
}

void OpThreadTaskProcessor::startTask(std::function<void()> f)
{
    _opthreads->add(TaskOperation::create(f));
}

