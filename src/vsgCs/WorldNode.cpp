#include "WorldNode.h"

#include "CsOverlay.h"
#include "jsonUtils.h"
#include "RuntimeEnvironment.h"
#include "Styling.h"
#include "TilesetNode.h"


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
    vsg::ref_ptr<TilesetNode> makeTilesetNode(const rapidjson::Value& tsObject, JSONObjectFactory* factory)
    {
        return ref_ptr_cast<TilesetNode>(factory->build(tsObject));
    }
}

void WorldNode::init(const rapidjson::Value& worldJson, JSONObjectFactory* factory)
{
    if (!factory)
    {
        factory = JSONObjectFactory::get();
    }
    auto tilesetParent = ref_ptr_cast<vsg::StateGroup>(children[0]);
    auto tilesetsItr = worldJson.FindMember("tilesets");
    if (tilesetsItr == worldJson.MemberEnd() || !tilesetsItr->value.IsArray())
    {
        vsg::error("No tilesets array in json");
        return;
    }
    const auto& tilesets = tilesetsItr->value;
    for (rapidjson::SizeType i = 0; i < tilesets.Size(); ++i)
    {
        auto tilesetNode = makeTilesetNode(tilesets[i].GetObject(), factory);
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

namespace
{
    vsg::ref_ptr<vsg::Object> buildWorldNode(const rapidjson::Value& json,
                                               JSONObjectFactory* factory,
                                               vsg::ref_ptr<vsg::Object> object)
    {
        if (!object)
        {
            object = WorldNode::create();
        }
        auto world = ref_ptr_cast<WorldNode>(object);
        world->init(json, factory);
        return world;
    }

    JSONObjectFactory::Registrar r("World", buildWorldNode);
}
