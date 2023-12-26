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

#pragma once

#include <vsg/all.h>

#include <CesiumGltf/Model.h>
#include <Cesium3DTilesSelection/Tile.h>
#include <glm/gtc/type_ptr.hpp>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorSetLayout.h>

#include "Export.h"
#include "GraphicsEnvironment.h"
#include "ModelBuilder.h"
#include "RuntimeEnvironment.h"
#include "runtimeSupport.h"


#include <array>

// Build a VSG scenegraph from a Cesium Gltf Model object.

namespace vsgCs
{
    // The functions for attaching and detaching rasters return objects that need to be compiled and
    // may replace objects that should then eventually be freed. I don't think the tile-building
    // commands need this generality.
    struct ModifyRastersResult
    {
        std::vector<vsg::ref_ptr<vsg::Object>> compileObjects;
        std::vector<vsg::ref_ptr<vsg::Object>> deleteObjects;
    };

    // attachTileData(), called by prepareInMainThread(), returns both a descriptor set that has
    // been compiled as well as possibly updated model tile (with a tile bounding volume).
    struct AttachTileDataResult
    {
        vsg::ref_ptr<vsg::Object> descriptorData;
        vsg::ref_ptr<vsg::Node> updatedModel;
    };

    // Interface from Cesium Native to the VSG scene graph. CesiumGltfBuilder can load Models (glTF
    // assets) and images that are not part of a model. The exact type of the VSG scene graph node
    // is isolated from the rest of vsgCs. The only functions that should care -- i.e., that modify the
    // tile subscene graph -- are here in CesiumGltfBuilder.

    class VSGCS_EXPORT CesiumGltfBuilder : public vsg::Inherit<vsg::Object, CesiumGltfBuilder>
    {
    public:
        CesiumGltfBuilder(const vsg::ref_ptr<GraphicsEnvironment>& genv);
        vsg::ref_ptr<vsg::Group> load(CesiumGltf::Model* model, const CreateModelOptions& options);
        vsg::ref_ptr<vsg::Node> loadTile(Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
                                         const glm::dmat4& transform,
                                         const CreateModelOptions& options);
        AttachTileDataResult attachTileData(Cesium3DTilesSelection::Tile& tile,
                                            const vsg::ref_ptr<vsg::Node>& node);
        vsg::ref_ptr<vsg::ImageInfo> loadTexture(CesiumGltf::ImageCesium& image,
                                                 VkSamplerAddressMode addressX,
                                                 VkSamplerAddressMode addressY,
                                                 VkFilter minFilter,
                                                 VkFilter maxFilter,
                                                 bool useMipMaps,
                                                 bool sRGB);
        vsg::ref_ptr<vsg::ImageInfo> loadTexture(CesiumTextureSource&& imageSource,
                                                 VkSamplerAddressMode addressX,
                                                 VkSamplerAddressMode addressY,
                                                 VkFilter minFilter,
                                                 VkFilter maxFilter,
                                                 bool useMipMaps,
                                                 bool sRGB);

        ModifyRastersResult attachRaster(const Cesium3DTilesSelection::Tile& tile,
                                         const vsg::ref_ptr<vsg::Node>& node,
                                         int32_t overlayTextureCoordinateID,
                                         const CesiumRasterOverlays::RasterOverlayTile& rasterTile,
                                         void* pMainThreadRendererResources,
                                         const glm::dvec2& translation,
                                         const glm::dvec2& scale);
        ModifyRastersResult detachRaster(const Cesium3DTilesSelection::Tile& tile,
                                         const vsg::ref_ptr<vsg::Node>& node,
                                         int32_t overlayTextureCoordinateID,
                                         const CesiumRasterOverlays::RasterOverlayTile& rasterTile);
        static vsg::ref_ptr<vsg::StateGroup> getTileStateGroup(const vsg::ref_ptr<vsg::Node>& node);
        static vsg::ref_ptr<vsg::Data> getTileData(const vsg::ref_ptr<vsg::Node>& node);
    protected:
        vsg::ref_ptr<GraphicsEnvironment> _genv;
    };

}

