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

#include "accessor_traits.h"
#include "CesiumGltfBuilder.h"
#include "pbr.h"
#include "DescriptorSetConfigurator.h"
#include "MultisetPipelineConfigurator.h"
#include "LoadGltfResult.h"
#include "runtimeSupport.h"

// Some include file in Cesium (actually, it's spdlog.h) unleashes the hell of windows.h. We need to
// turn off GDI defines to avoid a redefinition of the GLSL constant OPAQUE.
#ifndef NOGDI
#define NOGDI
#endif

#include <CesiumGltf/AccessorView.h>
#include <CesiumGltf/ExtensionKhrTextureBasisu.h>
#include <CesiumGltf/ExtensionTextureWebp.h>

#include <CesiumGltfReader/GltfReader.h>
#include <Cesium3DTilesSelection/GltfUtilities.h>
#include <Cesium3DTilesSelection/RasterOverlayTile.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>

#include <vsg/utils/ShaderSet.h>

#include <algorithm>
#include <string>
#include <tuple>
#include <limits>
#include <exception>


using namespace vsgCs;
using namespace CesiumGltf;

// Translate from a Cesium3dTilesSelection::BoundingVolume to a bounding sphere. Inspired by
// cesium-unreal.
struct BoundingSphereOperation
{

    vsg::dsphere operator()(const CesiumGeometry::BoundingSphere& sphere) const
    {
        return vsg::dsphere(glm2vsg(sphere.getCenter()), sphere.getRadius());
    }

    vsg::dsphere operator()(const CesiumGeometry::OrientedBoundingBox& box) const
    {
        vsg::dvec3 center = glm2vsg(box.getCenter());
        glm::dmat3 halfAxes = box.getHalfAxes();
        glm::dvec3 corner1 = halfAxes[0] + halfAxes[1];
        glm::dvec3 corner2 = halfAxes[0] + halfAxes[2];
        glm::dvec3 corner3 = halfAxes[1] + halfAxes[2];
        double sphereRadius = glm::max(glm::length(corner1), glm::length(corner2));
        sphereRadius = glm::max(sphereRadius, glm::length(corner3));
        return vsg::dsphere(center, sphereRadius);
    }

  vsg::dsphere operator()(const CesiumGeospatial::BoundingRegion& region) const
    {
        return (*this)(region.getBoundingBox());
    }

  vsg::dsphere operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region) const
    {
        return (*this)(region.getBoundingRegion());
    }

  vsg::dsphere operator()(const CesiumGeospatial::S2CellBoundingVolume& s2) const
    {
        return (*this)(s2.computeBoundingRegion());
    }
};

CesiumGltfBuilder::CesiumGltfBuilder(const vsg::ref_ptr<vsg::Options>& vsgOptions,
                                     const DeviceFeatures& deviceFeatures)
    : _genv(GraphicsEnvironment::create(vsgOptions, deviceFeatures))
{
}

CesiumGltfBuilder::CesiumGltfBuilder(const vsg::ref_ptr<GraphicsEnvironment>& genv)
    : _genv(genv)
{
}



// Hold Raster data that we can attach to the VSG tile in order to easily find
// it later.
//

namespace
{
    struct RasterData
    {
        std::string name;
        vsg::ref_ptr<vsg::ImageInfo> rasterImage;
        pbr::OverlayParams overlayParams;
    };

    struct Rasters : public vsg::Inherit<vsg::Object, Rasters>
    {
        explicit Rasters(size_t numOverlays)
            : overlayRasters(numOverlays)
        {
        }
        std::vector<RasterData> overlayRasters;
    };

    // Create a new Rasters object and store it in node if necessary
    vsg::ref_ptr<Rasters> getOrCreateRasters(const vsg::ref_ptr<vsg::Node>& node)
    {
        vsg::ref_ptr<Rasters> rasters(node->getObject<Rasters>("vsgCs_rasterData"));
        if (!rasters.valid())
        {
            rasters = Rasters::create(pbr::maxOverlays);
            node->setObject("vsgCs_rasterData", rasters);
        }
        return rasters;
    }
}

vsg::ref_ptr<vsg::StateCommand> makeTileStateCommand(const vsg::ref_ptr<GraphicsEnvironment>& genv,
                                                     const Rasters& rasters,
                                                     const Cesium3DTilesSelection::Tile& tile)
{
    vsg::ImageInfoList rasterImages(rasters.overlayRasters.size());
    // The topology doesn't matter because the pipeline layouts of shader versions are compatible.
    vsg::ref_ptr<DescriptorSetConfigurator> descriptorBuilder
        = DescriptorSetConfigurator::create(pbr::TILE_DESCRIPTOR_SET,
                                            genv->shaderFactory->getShaderSet(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST));
    std::vector<pbr::OverlayParams> overlayParams(rasters.overlayRasters.size());
    for (size_t i = 0; i < rasters.overlayRasters.size(); ++i)
    {
        const auto& rasterData = rasters.overlayRasters[i];
        rasterImages[i] = rasterData.rasterImage.valid()
            ? rasterData.rasterImage : genv->defaultTexture;
        overlayParams[i] = rasterData.overlayParams;
    }
    descriptorBuilder->assignTexture("overlayTextures", rasterImages);
    auto ubo = pbr::makeTileData(tile.getGeometricError(), std::min(genv->features.pointSizeRange[1], 4.0f),
                                 overlayParams);
    descriptorBuilder->assignUniform("tileParams", ubo);
    descriptorBuilder->init();
    auto bindDescriptorSet
            = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                             genv->overlayPipelineLayout, pbr::TILE_DESCRIPTOR_SET,
                                             descriptorBuilder->descriptorSet);
    return bindDescriptorSet;
 }


vsg::ref_ptr<vsg::Group>
CesiumGltfBuilder::load(CesiumGltf::Model* model, const CreateModelOptions& options)
{
    static ExtensionList tilesExtensions{Cs3DTilesExtension::create()};

    ModelBuilder builder(_genv, model, options, tilesExtensions);
    return builder();
}

vsg::ref_ptr<vsg::StateGroup> CesiumGltfBuilder::getTileStateGroup(const vsg::ref_ptr<vsg::Node>& node)
{
    vsg::ref_ptr<vsg::MatrixTransform> transformNode;
    auto cullNode = ref_ptr_cast<vsg::CullNode>(node);
    if (cullNode.valid())
    {
        transformNode = ref_ptr_cast<vsg::MatrixTransform>(cullNode->child);
    }
    else
    {
        transformNode = ref_ptr_cast<vsg::MatrixTransform>(node);
    }
    if (!transformNode.valid())
    {
        vsg::warn("Weird tile graph");
        return {};
    }
    return ref_ptr_cast<vsg::StateGroup>(transformNode->children[0]);
}

vsg::ref_ptr<vsg::Node> CesiumGltfBuilder::loadTile(Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
                                                    const glm::dmat4 &transform,
                                                    const CreateModelOptions& modelOptions)
{
    CesiumGltf::Model* pModel = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);
    if (!pModel)
    {
        return {};
    }
    Model& model = *pModel;
    glm::dmat4x4 rootTransform = transform;

    rootTransform = Cesium3DTilesSelection::GltfUtilities::applyRtcCenter(model, rootTransform);
    rootTransform = Cesium3DTilesSelection::GltfUtilities::applyGltfUpAxisTransform(model, rootTransform);
    auto transformNode = vsg::MatrixTransform::create(glm2vsg(rootTransform));
    auto modelNode = load(pModel, modelOptions);
    auto tileStateGroup = vsg::StateGroup::create();
    auto bindViewDescriptorSets = vsg::BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                                      _genv->overlayPipelineLayout, 0);
    // Make uniforms (tile and raster parameters) and default textures for the tile.

    auto rasters = Rasters::create(pbr::maxOverlays);
    transformNode->setObject("vsgCs_rasterData", rasters);
    // For debugging
    auto it = model.extras.find("Cesium3DTiles_TileUrl");
    std::string url = it != model.extras.end()
        ? it->second.getStringOrDefault("Unknown Tile URL")
        : "Unknown Tile URL";
    transformNode->setValue("tileUrl", url);
    tileStateGroup->add(bindViewDescriptorSets);
    tileStateGroup->addChild(modelNode);
    transformNode->addChild(tileStateGroup);
    if (tileLoadResult.updatedBoundingVolume)
    {
        vsg::dsphere bounds = visit(BoundingSphereOperation(), *tileLoadResult.updatedBoundingVolume);
        auto cullNode = vsg::CullNode::create(bounds, transformNode);
        return cullNode;
    }
    return transformNode;
}

AttachTileDataResult
CesiumGltfBuilder::attachTileData(Cesium3DTilesSelection::Tile& tile,
                                  const vsg::ref_ptr<vsg::Node>& node)
{
    auto rasters = getOrCreateRasters(node);
    auto tileStateGroup = getTileStateGroup(node);
    auto tileStateCommand = makeTileStateCommand(_genv, *rasters, tile);
    tileStateGroup->add(tileStateCommand);
    // Were we able to create a CullNode for this tile?
    auto transformNode = ref_ptr_cast<vsg::MatrixTransform>(node);
    if (transformNode.valid())
    {
        vsg::dsphere bounds;
        if (tile.getContentBoundingVolume())
        {
            bounds = visit(BoundingSphereOperation(), *tile.getContentBoundingVolume());
        }
        else
        {
            bounds = visit(BoundingSphereOperation(), tile.getBoundingVolume());
        }
        auto cullNode = vsg::CullNode::create(bounds, transformNode);
        return {tileStateCommand, cullNode};
    }
    return {tileStateCommand, node};
}

vsg::ref_ptr<vsg::ImageInfo> CesiumGltfBuilder::loadTexture(CesiumTextureSource&& imageSource,
                                                            VkSamplerAddressMode addressX,
                                                            VkSamplerAddressMode addressY,
                                                            VkFilter minFilter,
                                                            VkFilter maxFilter,
                                                            bool useMipMaps,
                                                            bool sRGB)
{
    CesiumGltf::ImageCesium* pImage =
        std::visit(GetImageFromSource{}, imageSource);
    return loadTexture(*pImage, addressX, addressY, minFilter, maxFilter, useMipMaps, sRGB);
}

vsg::ref_ptr<vsg::ImageInfo> CesiumGltfBuilder::loadTexture(CesiumGltf::ImageCesium& image,
                                                            VkSamplerAddressMode addressX,
                                                            VkSamplerAddressMode addressY,
                                                            VkFilter minFilter,
                                                            VkFilter maxFilter,
                                                            bool useMipMaps,
                                                            bool sRGB)
{
    auto data = loadImage(image, useMipMaps, sRGB);
    if (!data)
    {
        return {};
    }
    auto sampler = makeSampler(addressX, addressY, minFilter, maxFilter,
                               samplerLOD(data, useMipMaps));
    _genv->sharedObjects->share(sampler);
    return vsg::ImageInfo::create(sampler, data);
}


ModifyRastersResult CesiumGltfBuilder::attachRaster(const Cesium3DTilesSelection::Tile& tile,
                                                    const vsg::ref_ptr<vsg::Node>& node,
                                                    int32_t overlayTextureCoordinateID,
                                                    const Cesium3DTilesSelection::RasterOverlayTile&,
                                                    void* pMainThreadRendererResources,
                                                    const glm::dvec2& translation,
                                                    const glm::dvec2& scale)
{
    ModifyRastersResult result;
    vsg::ref_ptr<Rasters> rasters = getOrCreateRasters(node);
    vsg::ref_ptr<vsg::StateGroup> stateGroup = getTileStateGroup(node);
    if (!stateGroup)
    {
        return {};
    }
    RasterResources *resource = static_cast<RasterResources*>(pMainThreadRendererResources);
    auto raster = resource->raster;
    auto& rasterData = rasters->overlayRasters.at(resource->overlayOptions.layerNumber);
    rasterData.rasterImage = raster;
    rasterData.overlayParams.translation = glm2vsg(translation);
    rasterData.overlayParams.scale = glm2vsg(scale);
    rasterData.overlayParams.coordIndex = overlayTextureCoordinateID;
    rasterData.overlayParams.enabled = 1;
    rasterData.overlayParams.alpha = resource->overlayOptions.alpha;
    auto command = makeTileStateCommand(_genv, *rasters, tile);
    // XXX Should check data or something in the state command instead of relying on the number of
    // commands in the group.
    auto& stateCommands = stateGroup->stateCommands;
    if (stateCommands.size() > 1)
    {
        result.deleteObjects.insert(result.deleteObjects.end(),
                                    stateCommands.begin() + 1, stateCommands.end());
        stateCommands.erase(stateCommands.begin() + 1, stateCommands.end());
    }
    stateCommands.push_back(command);
    result.compileObjects.push_back(command);
    return result;
}

ModifyRastersResult
CesiumGltfBuilder::detachRaster(const Cesium3DTilesSelection::Tile& tile,
                                const vsg::ref_ptr<vsg::Node>& node,
                                int32_t,
                                const Cesium3DTilesSelection::RasterOverlayTile& rasterTile)
{
    ModifyRastersResult result;
    vsg::ref_ptr<Rasters> rasters = getOrCreateRasters(node);
    vsg::ref_ptr<vsg::StateGroup> stateGroup = getTileStateGroup(node);
    if (!stateGroup)
    {
        return {};
    }
    RasterResources *resource = static_cast<RasterResources*>(rasterTile.getRendererResources());
    auto& rasterData = rasters->overlayRasters.at(resource->overlayOptions.layerNumber);
    {
        rasterData.rasterImage = {}; // ref to rasterImage is still held by the old StateCommand
        rasterData.overlayParams.enabled = 0;
        auto newCommand = makeTileStateCommand(_genv, *rasters, tile);
        vsg::ref_ptr<vsg::Command> oldCommand;
        // XXX Should check data or something in the state command instead of relying on the number of
        // commands in the group.
        auto& stateCommands = stateGroup->stateCommands;
        if (stateCommands.size() > 1)
        {
            oldCommand = *(stateCommands.begin() + 1);
            stateCommands.erase(stateCommands.begin() + 1);
        }
        stateCommands.push_back(newCommand);
        result.compileObjects.push_back(newCommand);
        result.deleteObjects.push_back(oldCommand);
    }
    return result;
}
