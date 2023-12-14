#include "TracingCommandGraph.h"

#include "RuntimeEnvironment.h"

#include <vsg/app/View.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/ui/FrameStamp.h>
#include <vsg/vk/State.h>

namespace vsgCs
{
    TracingCollectCommand::TracingCollectCommand(vsg::ref_ptr<TracyContextValue> in_tracyCtx)
        : tracyCtx(in_tracyCtx)
    {
    }

    void TracingCollectCommand::record(vsg::CommandBuffer& cmd) const
    {
#ifdef TRACY_ENABLE
        if (tracyCtx && tracyCtx->gpuProfilingMask != 0)
        {
            TracyVkCollect(tracyCtx->ctx, cmd.vk());
        }
#endif
        (void)cmd;
    }
    
    TracingCommandGraph::TracingCommandGraph(const vsg::ref_ptr<RuntimeEnvironment>& in_env,
                                             vsg::ref_ptr<vsg::Window> in_window,
                                             vsg::ref_ptr<vsg::Node> child)
        : Inherit(in_window, child), features(in_env->features), tracyCtx(in_env->tracyContext)
    {
        addChild(TracingCollectCommand::create(tracyCtx));
    }

    void TracingCommandGraph::record(vsg::ref_ptr<vsg::RecordedCommandBuffers> recordedCommandBuffers,
                                     vsg::ref_ptr<vsg::FrameStamp> frameStamp,
                                     vsg::ref_ptr<vsg::DatabasePager> databasePager)
    {
#ifdef TRACY_ENABLE
        if (!tracyCtx->ctx)
        {
            if (window && !window->visible())
            {
                return;
            }
            getOrCreateRecordTraversal();
            vsg::ref_ptr<vsg::CommandPool> cp = vsg::CommandPool::create(device, queueFamily,
                                                                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            auto tmpCmd = cp->allocate(level());
            auto queue = device->getQueue(queueFamily, 0);
            if (features.vkGetCalibratedTimestampsEXT)
            {
                tracyCtx->ctx
                    = TracyVkContextCalibrated(device->getPhysicalDevice()->vk(), device->vk(), queue->vk(),
                                               tmpCmd->vk(),
                                               features.vkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
                                               features.vkGetCalibratedTimestampsEXT);
            }
            else
            {
                tracyCtx->ctx = TracyVkContext(device->getPhysicalDevice()->vk(), device->vk(), queue->vk(),
                                           tmpCmd->vk());
            }
            recordTraversal->setObject("tracyCtx", tracyCtx);
            auto collectCmd = children[0].cast<TracingCollectCommand>();
            if (collectCmd)
            {
                collectCmd->tracyCtx = tracyCtx;
            }
        }
#endif
        Inherit::record(recordedCommandBuffers, frameStamp, databasePager);
    }

    void TracingVertexDraw::accept(vsg::RecordTraversal& visitor) const
    {
#ifdef TRACY_ENABLE
        auto tracyCtx = visitor.getObject<TracyContextValue>("tracyCtx");
        TracyVkNamedZone(tracyCtx->ctx, vertexDraw, visitor.getState()->_commandBuffer->vk(),
                         "VertexDraw", tracyCtx->gpuProfilingMask != 0);
#endif
        VertexDraw::accept(visitor);
    }

    void TracingVertexIndexDraw::accept(vsg::RecordTraversal& visitor) const
    {
#ifdef TRACY_ENABLE
        auto tracyCtx = visitor.getObject<TracyContextValue>("tracyCtx");
        TracyVkNamedZone(tracyCtx->ctx, vertexIndexDraw, visitor.getState()->_commandBuffer->vk(),
                         "VertexIndexDraw", tracyCtx->gpuProfilingMask != 0);
#endif
        VertexIndexDraw::accept(visitor);
    }

    TracingRenderGraph::TracingRenderGraph(vsg::ref_ptr<vsg::Window> in_window,
                                                vsg::ref_ptr<vsg::View> view)
        : Inherit(in_window, view)
    {
    }

    void TracingRenderGraph::accept(vsg::RecordTraversal& visitor) const
    {
#ifdef TRACY_ENABLE
        auto tracyCtx = visitor.getObject<TracyContextValue>("tracyCtx");
        TracyVkNamedZone(tracyCtx->ctx, renderGraph, visitor.getState()->_commandBuffer->vk(),
                         "RenderGraph", tracyCtx->gpuProfilingMask != 0);
#endif
        RenderGraph::accept(visitor);
    }
}
