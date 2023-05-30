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
    explicit TaskOperation(std::function<void()> f)
        : _f(std::move(f))
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

