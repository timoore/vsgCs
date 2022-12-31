#pragma once

#include <cstdint>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/maths/vec2.h>
#include <vsg/utils/ShaderSet.h>

namespace vsgCs
{
    namespace pbr
    {
        // The overlay uniform structure.
        struct OverlayParams
        {
            vsg::vec2 translation;
            vsg::vec2 scale;
            uint32_t enabled;
            uint32_t coordIndex;
            uint32_t pad[2];
        };
        typedef OverlayParams OverlayUniformMem[2];
        vsg::ref_ptr<vsg::ShaderSet> makeShaderSet(vsg::ref_ptr<const vsg::Options> options = {});
    }

}
