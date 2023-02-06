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
        enum DescriptorSetUse : uint32_t
        {
            VIEW_DESCRIPTOR_SET = 0,
            TILESET_DESCRIPTOR_SET = 1,
            TILE_DESCRIPTOR_SET = 2,
            PRIMITIVE_DESCRIPTOR_SET = 3
        };
        // The overlay uniform structure.
        struct OverlayParams
        {
            OverlayParams()
                : alpha(1.0f), enabled(0), coordIndex(0)
            {
                pad[0] = 0;
            }
            vsg::vec2 translation;
            vsg::vec2 scale;
            float alpha;
            uint32_t enabled;
            uint32_t coordIndex;
            uint32_t pad[1];
        };

        // This will eventually not be constant, but it will be a major event to change it at
        // runtime.
        const unsigned maxOverlays = 4;

        vsg::ref_ptr<vsg::Data> makeTileData(float geometricError, float maxPointSize,
                                             const gsl::span<OverlayParams> overlayUniformMem);
        vsg::ref_ptr<vsg::ShaderSet> makeShaderSet(vsg::ref_ptr<const vsg::Options> options = {});
        vsg::ref_ptr<vsg::ShaderSet> makePointShaderSet(vsg::ref_ptr<const vsg::Options> options = {});
    }

}
