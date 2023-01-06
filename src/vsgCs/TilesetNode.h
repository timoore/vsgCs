#pragma once

#include <vsg/all.h>
#include "Cesium3DTilesSelection/Tileset.h"
#include "Cesium3DTilesSelection/ViewUpdateResult.h"
#include "vsgResourcePreparer.h"

#include <memory>
#include <optional>
#include <string>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>

namespace vsgCs
{
    struct VSGCS_EXPORT TilesetDeviceFeatures
    {
        bool indexTypeUint8 = false;
        bool textureCompressionETC2 = false;
        bool textureCompressionASTC_LDR = false;
        bool textureCompressionBC = false;
        bool textureCompressionPVRTC = false;
    };

    struct VSGCS_EXPORT TilesetSource
    {
        std::optional<std::string> url;
        std::optional<int64_t> ionAssetID;
        std::optional<std::string> ionAccessToken;
        std::optional<std::string> ionAssetEndpointUrl;
    };
    
    class VSGCS_EXPORT TilesetNode : public vsg::Inherit<vsg::Node, TilesetNode>
    {
    public:
        static vsg::dmat4 zUp2yUp;
        static vsg::dmat4 yUp2zUp;
    
        TilesetNode(const TilesetDeviceFeatures& deviceFeatures, const TilesetSource& source,
                    const Cesium3DTilesSelection::TilesetOptions& tilesetOptions,
                    vsg::ref_ptr<vsg::Options> options);
        ~TilesetNode() override;
        /**
         * @brief Set up the window traits to create the Vulkan Device with the desired features, etc.,
         * @returns features that are actually available for Cesium and vsgCs.
         */
        static TilesetDeviceFeatures prepareDeviceFeatures(vsg::ref_ptr<vsg::Window> window);
        /**
         * @brief Load the tileset, set up recurring VSG tasks, and perform any other necessary
         * initialization. This calls updateViews().
         *
         * Call this after the initial command graphs have been assigned to the viewer, but before
         * the main loop starts.
         */
        bool initialize(vsg::ref_ptr<vsg::Viewer> viewer);
        /**
         * @brief Call when cameras and views are added to the viewer
         */
        void updateViews(vsg::ref_ptr<vsg::Viewer> viewer);
        // void attachToViewer(vsg::ref_ptr<vsg::Viewer> viewer, vsg::ref_ptr<vsg::Group> attachment);
        void traverse(vsg::Visitor& visitor) override;
        void traverse(vsg::ConstVisitor& visitor) const override;
        void traverse(vsg::RecordTraversal& visitor) const override;
        struct UpdateTileset : public vsg::Inherit<vsg::Operation, UpdateTileset>
        {
            UpdateTileset(vsg::ref_ptr<TilesetNode> in_tilesetNode, vsg::ref_ptr<vsg::Viewer> in_viewer)
                : tilesetNode(in_tilesetNode), viewer(in_viewer)
            {}
            void run() override;
            vsg::observer_ptr<TilesetNode> tilesetNode;
            vsg::observer_ptr<vsg::Viewer> viewer;
        };
        friend struct UpdateTileset;
        void shutdown();
    protected:
        const Cesium3DTilesSelection::ViewUpdateResult* _viewUpdateResult;
        std::unique_ptr<Cesium3DTilesSelection::Tileset> _tileset;
        std::shared_ptr<vsgResourcePreparer> _resourcePreparer;
        std::shared_ptr<Cesium3DTilesSelection::CreditSystem> _creditSystem;
    private:
        template<class V> void t_traverse(V& visitor) const;
        int32_t _tilesetsBeingDestroyed;
        
    };
}
