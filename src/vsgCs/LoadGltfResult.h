// Heavily copied from Cesium Unreal, which is:
// Copyright 2020-2021 CesiumGS, Inc. and Contributors

#pragma once

#include "CesiumGltf/Material.h"
#include "CesiumGltf/MeshPrimitive.h"
#include "CesiumGltf/Model.h"

#include <cstdint>
#include <glm/mat4x4.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vsg/all.h>

// Cesium Unreal has all sorts of extra cruft in model results, primitive results, etc., but I think
// that's needed because of the strong load thread / game thread split in Unreal.

namespace vsgCs
{
    struct LoadModelResult
    {
        vsg::ref_ptr<vsg::Node> modelResult;
        vsg::CompileResult compileResult;
    };

    // Reference to model that is kept in a Cesium Tile as a pointer to void.

    struct RenderResources
    {
        vsg::ref_ptr<vsg::Node> model;
    };

    struct LoadRasterResult
    {
        vsg::ref_ptr<vsg::ImageInfo> rasterResult;
        vsg::CompileResult compileResult;
    };

    struct RasterResources
    {
        vsg::ref_ptr<vsg::ImageInfo> raster;
    };
}
