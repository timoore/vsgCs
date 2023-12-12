#pragma once

#include "vsgCs/Tracing.h"

#include <vsg/app/CommandGraph.h>
#include <vsg/commands/Command.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>

namespace vsgCs
{
    class TracingCollectCommand : public vsg::Inherit<vsg::Command, TracingCollectCommand>
    {
        public:
        TracingCollectCommand(vsg::ref_ptr<TracyContextValue> in_tracyCtx);
        void record(vsg::CommandBuffer& cmd) const override;
        vsg::ref_ptr<TracyContextValue> tracyCtx;
    };
    
    class TracingCommandGraph : public vsg::Inherit<vsg::CommandGraph, TracingCommandGraph>
    {
        public:
        explicit TracingCommandGraph(vsg::ref_ptr<vsg::Window> in_window, vsg::ref_ptr<vsg::Node> child = {});
        void record(vsg::ref_ptr<vsg::RecordedCommandBuffers> recordedCommandBuffers, vsg::ref_ptr<vsg::FrameStamp> frameStamp, vsg::ref_ptr<vsg::DatabasePager> databasePager) override;

        vsg::ref_ptr<TracyContextValue> tracyCtx;
        vsg::ref_ptr<vsg::CommandBuffer> tmpCmd;
    };

    class TracingVertexDraw : public vsg::Inherit<vsg::VertexDraw, TracingVertexDraw>
    {
    public:
        void accept(vsg::RecordTraversal& visitor) const override;
    };

    class TracingVertexIndexDraw : public vsg::Inherit<vsg::VertexIndexDraw, TracingVertexIndexDraw>
    {
        public:
        void accept(vsg::RecordTraversal& visitor) const override;
    };
    
}

