#include "WorldNode.h"

#include "RuntimeEnvironment.h"
#include "TilesetNode.h"
#include "vsgCs/CSOverlay.h"

#include <vsg/all.h>
#include <rapidjson/document.h>

#include <CesiumUtility/JsonHelpers.h>
#include <stdexcept>
#include <algorithm>

using namespace vsgCs;

WorldNode::WorldNode()
{
    // This state group will hold the TilesetNode objects.
    auto stateGroup = vsg::StateGroup::create();
    addChild(stateGroup);
}

namespace
{
    vsg::ref_ptr<TilesetNode> makeTilesetNode(const rapidjson::Value& tsObject)
    {
        auto env = RuntimeEnvironment::get();
        auto ionAsset = CesiumUtility::JsonHelpers::getInt64OrDefault(tsObject, "ionAssetID", -1);
        auto ionOverlay = CesiumUtility::JsonHelpers::getInt64OrDefault(tsObject, "ionOverlayID", -1);
        auto tilesetUrl = CesiumUtility::JsonHelpers::getStringOrDefault(tsObject, "tilesetUrl", "");
        auto ionAccessToken = CesiumUtility::JsonHelpers::getStringOrDefault(tsObject, "ionAccessToken", "");
        auto ionEndpointUrl = CesiumUtility::JsonHelpers::getStringOrDefault(tsObject, "ionEndpointUrl", "");
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
        if (ionOverlay > 0)
        {
            auto csoverlay = vsgCs::CSIonRasterOverlay::create(ionOverlay, ionAccessToken);
            csoverlay->addToTileset(tilesetNode);
        }

        return tilesetNode;
    }
}

void WorldNode::init(gsl::span<const char> data)
{
    auto tilesetParent = ref_ptr_cast<vsg::StateGroup>(children[0]);
    rapidjson::Document worldJson;
    worldJson.Parse(data.data(), data.size());
    if (worldJson.HasParseError())
    {
        vsg::error("Error parsing json: error code ", worldJson.GetParseError(),
                   " at byte ", worldJson.GetErrorOffset(),
                   "\n Source:\n",
                   std::string(data.data(), data.data() + std::min(128ul, data.size())));
        return;
    }
    auto tilesetsItr = worldJson.FindMember("tilesets");
    if (tilesetsItr == worldJson.MemberEnd() || !tilesetsItr->value.IsArray())
    {
        vsg::error("No tilesets array in json");
        return;
    }
    const auto& tilesets = tilesetsItr->value;
    for (rapidjson::SizeType i = 0; i < tilesets.Size(); ++i)
    {
        auto tilesetNode = makeTilesetNode(tilesets[i].GetObject());
        tilesetParent->addChild(tilesetNode);
    }
}

bool WorldNode::initialize(vsg::ref_ptr<vsg::Viewer> viewer)
{
    bool result = true;
    for (auto node : worldNodes())
    {
        auto tilesetNode = ref_ptr_cast<TilesetNode>(node);
        if (!tilesetNode)
        {
            vsg::error("null tileset node");
            result = false;
        }
        else
        {
            result &= tilesetNode->initialize(viewer);
        }
    }
    return result;
}

void WorldNode::shutdown()
{
    for (auto node : worldNodes())
    {
        auto tilesetNode = ref_ptr_cast<TilesetNode>(node);
        if (!tilesetNode)
        {
            vsg::error("null tileset node");
        }
        else
        {
            tilesetNode->shutdown();
        }
    }
}

const Cesium3DTilesSelection::Tile* WorldNode::getRootTile(size_t tileset)
{
    auto tilesetNode = ref_ptr_cast<TilesetNode>(worldNodes().at(tileset));
    if (tilesetNode)
    {
        return tilesetNode->getTileset()->getRootTile();
    }
    return nullptr;
}
