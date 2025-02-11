/* <editor-fold desc="MIT License">

Copyright(c) 2025 Timothy Moore

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

#include "GraphicsEnvironment.h"

#include <vsg/commands/Command.h>

#include <string>
#include <utility>

namespace vsgCs
{
    class BeginLabel : public vsg::Inherit<vsg::Command, BeginLabel>
    {
    public:
        BeginLabel(const vsg::ref_ptr<GraphicsEnvironment>& in_env, std::string in_label)
            : label(std::move(in_label)), env(in_env)
        {
        }
        void record(vsg::CommandBuffer& commandBuffer) const override;
        std::string label;
    protected:
        vsg::ref_ptr<GraphicsEnvironment> env;
    };

    class EndLabel : public vsg::Inherit<vsg::Command, EndLabel>
    {
    public:
        explicit EndLabel(const vsg::ref_ptr<GraphicsEnvironment>& in_env)
            : env(in_env)
        {
        }
        void record(vsg::CommandBuffer& commandBuffer) const override;
    protected:
        vsg::ref_ptr<GraphicsEnvironment> env;
    };
}
