#include "runtimeSupport.h"

#include <Cesium3DTilesSelection/registerAllTileContentTypes.h>
#include <Cesium3DTilesSelection/Tile.h>

namespace vsgCs
{
    using namespace Cesium3DTilesSelection;
    void startup()
    {
        registerAllTileContentTypes();
    }

    vsg::ref_ptr<vsg::LookAt> makeLookAtFromTile(Cesium3DTilesSelection::Tile* tile,
                                                 double distance)
    {
        auto boundingVolume = tile->getBoundingVolume();
        auto boundingRegion = getBoundingRegionFromBoundingVolume(boundingVolume);
        if (boundingRegion)
        {
            auto cartoCenter = boundingRegion->getRectangle().computeCenter();
            // The geographic coordinates specify a normal to the ellipsoid. How convenient!
            auto normal = CesiumGeospatial::Ellipsoid::WGS84.geodeticSurfaceNormal(cartoCenter);
            auto position = CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(cartoCenter);
            auto vNormal = glm2vsg(normal);
            auto vPosition = glm2vsg(position);
            auto geoCenter = glm2vsg(getBoundingVolumeCenter(boundingVolume));
            if (distance == std::numeric_limits<double>::max())
            {
                distance = vsg::length(vPosition - geoCenter) * 3.0;
            }
            vsg::dvec3 eye = vPosition +  vNormal * distance;
            vsg::dvec3 direction = -vNormal;
            // Try to align up with North
            auto side = vsg::cross(vsg::dvec3(0.0, 0.0, 1.0), direction);
            side = vsg::normalize(side);
            vsg::dvec3 up = vsg::cross(direction, side);
            return vsg::LookAt::create(eye, vPosition, up);
        }
        else
        {
            return vsg::LookAt::create();
        }
    }
}
