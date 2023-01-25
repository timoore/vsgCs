#pragma once

#include <cstdint>

#include <gsl/span>

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
            OverlayParams()
                : enabled(0), coordIndex(0)
            {
                pad[0] = 0;
                pad[1] = 0;
            }
            vsg::vec2 translation;
            vsg::vec2 scale;
            uint32_t enabled;
            uint32_t coordIndex;
            uint32_t pad[2];
        };

        // This will eventually not be constant, but it will be a major event to change it at
        // runtime.
        const unsigned maxOverlays = 4;

        vsg::ref_ptr<vsg::Data> makeOverlayData(const gsl::span<OverlayParams> overlayUniformMem);
        vsg::ref_ptr<vsg::ShaderSet> makeShaderSet(vsg::ref_ptr<const vsg::Options> options = {});
    }

}
