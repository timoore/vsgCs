#pragma once

#include "vsgCs/Tracing.h"
#include "vsgCs/RuntimeEnvironment.h"

#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
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
        explicit TracingCommandGraph(const vsg::ref_ptr<RuntimeEnvironment>& in_env, vsg::ref_ptr<vsg::Window> in_window, vsg::ref_ptr<vsg::Node> child = {});
        void record(vsg::ref_ptr<vsg::RecordedCommandBuffers> recordedCommandBuffers, vsg::ref_ptr<vsg::FrameStamp> frameStamp, vsg::ref_ptr<vsg::DatabasePager> databasePager) override;
        const DeviceFeatures features;
        vsg::ref_ptr<TracyContextValue> tracyCtx;
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

    class TracingRenderGraph : public vsg::Inherit<vsg::RenderGraph, TracingRenderGraph>
    {
    public:
        explicit TracingRenderGraph(vsg::ref_ptr<vsg::Window> in_window, vsg::ref_ptr<vsg::View> view = {});
        void accept(vsg::RecordTraversal& visitor) const override;
    };
    
}

