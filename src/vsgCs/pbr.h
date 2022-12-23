#pragma once

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/utils/ShaderSet.h>

namespace vsgCs
{
    vsg::ref_ptr<vsg::ShaderSet> pbr_ShaderSet(vsg::ref_ptr<const vsg::Options> options = {});
}
