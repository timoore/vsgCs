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

#include "runtimeSupport.h"
#include "RuntimeEnvironment.h"
#include "OpThreadTaskProcessor.h"

#include <Cesium3DTilesSelection/registerAllTileContentTypes.h>
#include <Cesium3DTilesSelection/Tile.h>

namespace vsgCs
{
    using namespace Cesium3DTilesSelection;
    void startup()
    {
        registerAllTileContentTypes();
    }

    void shutdown()
    {
        getAsyncSystemWrapper().shutdown();
    }

    vsg::ref_ptr<vsg::LookAt> makeLookAtFromTile(const Cesium3DTilesSelection::Tile* tile,
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

    std::string readFile(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options)
    {
        if (!options)
        {
            options = RuntimeEnvironment::get()->options;
        }
        vsg::Path filePath = vsg::findFile(filename, options);
        if (filePath.empty())
        {
            throw std::runtime_error("Could not find " + filename);
        }
        std::fstream f(filePath);
        std::stringstream iss;
        iss << f.rdbuf();
        return iss.str();
    }

    std::optional<uint32_t> getUintSuffix(const std::string& prefix, const std::string& data)
    {
        if (prefix.size() >= data.size())
        {
            return {};
        }
        auto match = std::mismatch(prefix.begin(), prefix.end(), data.begin());
        if (match.first == prefix.end())
        {
            long val = std::strtol(&(*match.second), nullptr, 10);
            return static_cast<uint32_t>(val);
        }
        return {};
    }
}
