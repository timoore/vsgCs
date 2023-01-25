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

#include "TilesetNode.h"

#include "CSOverlay.h"
#include "jsonUtils.h"
#include "OpThreadTaskProcessor.h"
#include "UrlAssetAccessor.h"

#include <CesiumUtility/JsonHelpers.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <optional>
#include <cmath>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>

using namespace vsgCs;
using namespace CesiumGltf;

template<typename F>
void for_each_view(vsg::ref_ptr<vsg::Viewer> viewer, const F& f)
{
    for (auto rsTasksItr = viewer->recordAndSubmitTasks.begin();
         rsTasksItr != viewer->recordAndSubmitTasks.end();
         ++rsTasksItr)
    {
        for (auto cgItr = (*rsTasksItr)->commandGraphs.begin();
             cgItr != (*rsTasksItr)->commandGraphs.end();
             ++cgItr)
        {
            for (auto childItr = (*cgItr)->children.begin();
                 childItr != (*cgItr)->children.end();
                ++childItr)
            {
                vsg::ref_ptr<vsg::RenderGraph> rg(dynamic_cast<vsg::RenderGraph*>(childItr->get()));
                if (rg)
                {
                    for (auto rgChildItr = rg->children.begin();
                         rgChildItr != rg->children.end();
                         ++rgChildItr)
                    {
                        vsg::ref_ptr<vsg::View> view(dynamic_cast<vsg::View*>(rgChildItr->get()));
                        if (view)
                        {
                            f(view, rg);
                        }
                    }
                }
            }
        }
    }
}

TilesetNode::TilesetNode(const DeviceFeatures& deviceFeatures, const TilesetSource& source,
                         const Cesium3DTilesSelection::TilesetOptions& in_options,
                         vsg::ref_ptr<vsg::Options> vsgOptions)
    : _viewUpdateResult(nullptr), _tilesetsBeingDestroyed(0)
{
    Cesium3DTilesSelection::TilesetOptions options(in_options);
    // turn off all the unsupported stuff
    options.enableOcclusionCulling = false;
    // Generous per-frame time limits for loading / unloading on main thread.
    options.mainThreadLoadingTimeLimit = 5.0;
    options.tileCacheUnloadTimeLimit = 5.0;
    options.contentOptions.enableWaterMask = false;
    options.loadErrorCallback =
        [this](const Cesium3DTilesSelection::TilesetLoadFailureDetails& details)
        {
            assert(this->_tileset.get() == details.pTileset);
            vsg::warn(details.message);
        };
    std::shared_ptr<CesiumAsync::IAssetAccessor> assetAccessor = std::make_shared<UrlAssetAccessor>();
    const CesiumAsync::AsyncSystem& asyncSystem = getAsyncSystem();
    _resourcePreparer = std::make_shared<vsgResourcePreparer>(vsgOptions);
    _creditSystem = std::make_shared<Cesium3DTilesSelection::CreditSystem>();
    Cesium3DTilesSelection::TilesetExternals externals{assetAccessor, _resourcePreparer, asyncSystem,
        _creditSystem, spdlog::default_logger(), nullptr};
    CesiumGltf::SupportedGpuCompressedPixelFormats supportedFormats;
    if (deviceFeatures.textureCompressionETC2)
    {
        supportedFormats.ETC1_RGB = true;
        supportedFormats.ETC2_RGBA = true;
        supportedFormats.ETC2_EAC_R11 = true;
        supportedFormats.ETC2_EAC_RG11 = true;
    }
    if (deviceFeatures.textureCompressionBC)
    {
        supportedFormats.BC1_RGB = true;
        supportedFormats.BC3_RGBA = true;
        supportedFormats.BC4_R = true;
        supportedFormats.BC5_RG = true;
        supportedFormats.BC7_RGBA = true;
    }
    if (deviceFeatures.textureCompressionASTC_LDR)
    {
        supportedFormats.ASTC_4x4_RGBA = true;
    }
    if (deviceFeatures.textureCompressionPVRTC)
    {
        supportedFormats.PVRTC2_4_RGBA = true;
    }
    options.contentOptions.ktx2TranscodeTargets = CesiumGltf::Ktx2TranscodeTargets(supportedFormats, false);
    if (source.url)
    {
        _tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(externals, source.url.value(), options);
    }
    else
    {
        if (source.ionAssetEndpointUrl)
        {
            _tileset
                = std::make_unique<Cesium3DTilesSelection::Tileset>(externals,
                                                                    source.ionAssetID.value(),
                                                                    source.ionAccessToken.value(),
                                                                    options,
                                                                    source.ionAssetEndpointUrl.value());
        }
        else
        {
            _tileset
                = std::make_unique<Cesium3DTilesSelection::Tileset>(externals,
                                                                    source.ionAssetID.value(),
                                                                    source.ionAccessToken.value(),
                                                                    options);
        }
    }
}

// From Cesium Unreal:
// Don't allow this Cesium3DTileset to be fully destroyed until
// any cesium-native Tilesets it created have wrapped up any async
// operations in progress and have been fully destroyed.
// See IsReadyForFinishDestroy.

void TilesetNode::shutdown()
{
    if (_tileset)
    {
        // Kind of gross, but the overlay is going to call TilesetNode::removeOverlay, which mutates
        // the _overlays vector.
        vsg::ref_ptr<TilesetNode> ref_this(this);
        while (!_overlays.empty())
        {
            (*(_overlays.end() - 1))->removeFromTileset(ref_this);
        }
        ++_tilesetsBeingDestroyed;
        _tileset->getAsyncDestructionCompleteEvent().thenInMainThread(
            [this]()
            {
                --this->_tilesetsBeingDestroyed;
            });
        _tileset.reset();
    }
}

TilesetNode::~TilesetNode()
{
    shutdown();
}

template<class V>
void TilesetNode::t_traverse(V& visitor) const
{
    if (_viewUpdateResult)
    {
        for (auto itr = _viewUpdateResult->tilesToRenderThisFrame.begin();
             itr != _viewUpdateResult->tilesToRenderThisFrame.end();
             ++itr)
        {
            const auto& tileContent = (*itr)->getContent();
            if (tileContent.isRenderContent())
            {
                const RenderResources *renderResources
                    = reinterpret_cast<const RenderResources*>(tileContent.getRenderContent()
                                                               ->getRenderResources());
                renderResources->model->accept(visitor);
            }
        }
    }
}

void TilesetNode::traverse(vsg::Visitor& visitor)
{
    t_traverse(visitor);
}

void TilesetNode::traverse(vsg::ConstVisitor& visitor) const
{
    t_traverse(visitor);
}

void TilesetNode::traverse(vsg::RecordTraversal& visitor) const
{
    t_traverse(visitor);
}

// We need to supply our cameras' poses (position, direction, up) to Cesium in its coordinate
// system i.e., Z up ECEF. The Cesium terrain may be attached to a VSG scenegraph with arbitrary
// transformations; at the very least, it should have a transformation to VSG Y up coordinates in
// its parents. If we follow the kinematic chain, The pose Pcs of the Cesium camera is
// 	Pcs = inv(Pw) * Pvsg * Tyup
// where Pw is the chain of transforms in the earth's parents, Pvsg is the pose of the VSG camera,
// and Tyup is the Z up to Y up transform.
// We actually have in hand the inverses of these matrices (pose is the inverse of a camera's view
// matrix), so we will calculate inv(Pcs) and invert it.

vsg::dmat4 TilesetNode::zUp2yUp(1.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, -1.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 1.0);

vsg::dmat4 TilesetNode::yUp2zUp(1.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                0.0, -1.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 1.0);

class FindNodeVisitor : public vsg::Inherit<vsg::Visitor, FindNodeVisitor>
{
public:
    FindNodeVisitor(vsg::ref_ptr<vsg::Node> node)
        : _node(node)
    {}
    FindNodeVisitor(vsg::Node* node)
        : _node(vsg::ref_ptr<vsg::Node>(node))
    {}
    void apply(vsg::Node& node) override
    {
        if (!resultPath.empty())
        {
            return;
        }
        _objectPath.push_back(&node);
        if (&node == _node.get())
        {
            resultPath.insert(resultPath.begin(), _objectPath.begin(), _objectPath.end());
        }
        else
        {
            node.traverse(*this);
        }
        _objectPath.pop_back();
    }
    
    vsg::RefObjectPath resultPath;
private:
    vsg::ref_ptr<vsg::Node> _node;
    vsg::ObjectPath _objectPath;
};

struct ViewData : public vsg::Inherit<vsg::Object, ViewData>
{
    ViewData(vsg::RefObjectPath& in_tilesetPath)
        : tilesetPath(in_tilesetPath)
    {}
    vsg::RefObjectPath tilesetPath;
};

namespace
{
    std::optional<Cesium3DTilesSelection::ViewState>
    createViewState(vsg::ref_ptr<vsg::View> view, vsg::ref_ptr<vsg::RenderGraph> renderGraph)
    {
        ViewData* viewData = dynamic_cast<ViewData*>(view->getObject("vsgCsViewData"));
        if (!viewData)
        {
            return {};
        }
        vsg::dmat4 Pw = vsg::computeTransform(viewData->tilesetPath);
        vsg::dmat4 PcsInv = TilesetNode::yUp2zUp * view->camera->viewMatrix->transform() * Pw;
        vsg::dmat4 Pcs = vsg::inverse(PcsInv);
        glm::dvec3 position(Pcs[3][0], Pcs[3][1], Pcs[3][2]);
        glm::dvec3 direction(Pcs[1][0], Pcs[1][1], Pcs[1][2]);
        glm::dvec3 up(Pcs[2][0], Pcs[2][1], Pcs[2][2]);
        // Have to assume that we have a perspective projection
        double fovy = 0.0;
        double fovx = 0.0;
        vsg::ref_ptr<vsg::ProjectionMatrix> projMat = view->camera->projectionMatrix;
        vsg::Perspective* persp = dynamic_cast<vsg::Perspective*>(projMat.get());
        if (persp)
        {
            fovy = vsg::radians(persp->fieldOfViewY);
            fovx = std::atan(std::tan(fovy / 2.0) * persp->aspectRatio) * 2.0;
        }
        else
        {
            vsg::dmat4 perspMat = projMat->transform();
            fovy = 2.0 * std::atan(-1.0 / perspMat[1][1]);
            fovx = 2.0 * std::atan(1.0 / perspMat[0][0]);
        }
        glm::dvec2 viewportSize;
        if (view->camera->viewportState)
        {
            VkViewport viewport = view->camera->viewportState->getViewport();
            viewportSize[0] = viewport.width;
            viewportSize[1] = viewport.height;
        }
        else
        {
            viewportSize[0] = renderGraph->renderArea.extent.width;
            viewportSize[1] = renderGraph->renderArea.extent.height;
        }
        Cesium3DTilesSelection::ViewState result =
            Cesium3DTilesSelection::ViewState::create(position, direction, up, viewportSize,
                                                      fovx, fovy);
        return std::optional<Cesium3DTilesSelection::ViewState>(result);
    }
}


    
void TilesetNode::updateViews(vsg::ref_ptr<vsg::Viewer> viewer)
{
    for_each_view(viewer,
                  [this](vsg::ref_ptr<vsg::View> view, vsg::ref_ptr<vsg::RenderGraph>)
                  {
                      FindNodeVisitor visitor(this);
                      view->accept(visitor);
                      if (visitor.resultPath.empty())
                      {
                          view->removeObject("vsgCsViewData");
                      }
                      else
                      {
                          view->setObject("vsgCsViewData", ViewData::create(visitor.resultPath));
                      }
                  });
}

void TilesetNode::UpdateTileset::run()
{
    vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
    vsg::ref_ptr<TilesetNode> ref_tileset = tilesetNode;

    if (!(ref_viewer && ref_tileset))
    {
        return;
    }
    if (!ref_tileset->_tileset)
    {
        return;
    }
    std::vector<Cesium3DTilesSelection::ViewState> viewStates;
    for_each_view(viewer,
                  [&viewStates](vsg::ref_ptr<vsg::View> view, vsg::ref_ptr<vsg::RenderGraph> rg)
                  {
                      auto viewState = createViewState(view, rg);
                      if (viewState)
                      {
                          viewStates.push_back(viewState.value());
                      }
                  });
    ref_tileset->_creditSystem->startNextFrame();
    ref_tileset->_viewUpdateResult = &ref_tileset->_tileset->updateView(viewStates);
}

bool TilesetNode::initialize(vsg::ref_ptr<vsg::Viewer> viewer)
{
    // XXX A bit gross initializing this here, but I think we want to allow creation of TilesetNode
    // before the viewer.
    _resourcePreparer->viewer = viewer;
    if (!viewer->compileManager)
    {
        vsg::warn("TilesetNode::initialize(): installing compile manager");
        viewer->compileManager = vsg::CompileManager::create(*viewer.get(),
                                                             vsg::ref_ptr<vsg::ResourceHints>());
    }
    updateViews(viewer);
    viewer->addUpdateOperation(UpdateTileset::create(vsg::ref_ptr<TilesetNode>(this), viewer),
                               vsg::UpdateOperations::ALL_FRAMES);
    return true;
}

void TilesetNode::addOverlay(vsg::ref_ptr<CSOverlay> overlay)
{
    _overlays.push_back(overlay);
    _tileset->getOverlays().add(overlay->getOverlay());
}

void TilesetNode::removeOverlay(vsg::ref_ptr<CSOverlay> overlay)
{
    _tileset->getOverlays().remove(overlay->getOverlay());
    _overlays.erase(std::remove(_overlays.begin(), _overlays.end(), overlay), _overlays.end());
}

namespace
{
    vsg::ref_ptr<vsg::Object> buildTilesetNode(const rapidjson::Value& json,
                                               JSONObjectFactory* factory,
                                               vsg::ref_ptr<vsg::Object>)
    {
        auto env = RuntimeEnvironment::get();
        // Current implementation of TilesetNode constructs Cesium tileset in the constructor, using
        // its arguments... so the object argument is not useful.
        // auto tilesetNode = create_or<TilesetNode>(object);
        auto ionAsset = CesiumUtility::JsonHelpers::getInt64OrDefault(json, "ionAssetID", -1);
        auto tilesetUrl = CesiumUtility::JsonHelpers::getStringOrDefault(json, "tilesetUrl", "");
        auto ionAccessToken = CesiumUtility::JsonHelpers::getStringOrDefault(json, "ionAccessToken", "");
        auto ionEndpointUrl = CesiumUtility::JsonHelpers::getStringOrDefault(json, "ionEndpointUrl", "");
        vsgCs::TilesetSource source;
        if (!tilesetUrl.empty())
        {
            source.url = tilesetUrl;
        }
        else
        {
            if (ionAsset < 0)
            {
                vsg::error("No valid Ion asset\n");
                return {};
            }
            source.ionAssetID = ionAsset;
            if (ionAccessToken.empty())
            {
                ionAccessToken = env->ionAccessToken;
            }
            source.ionAccessToken = ionAccessToken;
            if (!ionEndpointUrl.empty())
            {
                source.ionAssetEndpointUrl = ionEndpointUrl;
            }
        }
        Cesium3DTilesSelection::TilesetOptions tileOptions;
        tileOptions.enableOcclusionCulling = false;
        tileOptions.forbidHoles = true;
        auto tilesetNode = vsgCs::TilesetNode::create(env->features, source, tileOptions, env->options);
        const auto itr = json.FindMember("overlays");
        if (itr != json.MemberEnd() && itr->value.IsArray())
        {
            const auto& valueJson = itr->value;
            for (rapidjson::SizeType i = 0; i < valueJson.Size(); ++i)
            {
                const auto& element = valueJson[i];
                vsg::ref_ptr<CSOverlay> overlay = ref_ptr_cast<CSOverlay>(factory->build(element));
                if (!overlay)
                {
                    vsg::error("expected CSOverly, got ", overlay->className());
                    break;
                }
                overlay->layerNumber = i;
                overlay->addToTileset(tilesetNode);
            }
        }
        return tilesetNode;
    }

    JSONObjectFactory::Registrar r("Tileset", buildTilesetNode);
}
