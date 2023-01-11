#include <vsg/all.h>
#include <Cesium3DTilesSelection/Tile.h>
#include <glm/gtc/type_ptr.hpp>

#include "Export.h"
#include "vsgCs/Config.h"

// Various random helper functions

namespace vsgCs
{
    void VSGCS_EXPORT startup();
    void VSGCS_EXPORT shutdown();
    vsg::ref_ptr<vsg::LookAt> VSGCS_EXPORT makeLookAtFromTile(Cesium3DTilesSelection::Tile* tile,
                                                              double distance);

    inline void setdmat4(vsg::dmat4& vmat, const glm::dmat4x4& glmmat)
    {
        std::memcpy(vmat.data(), glm::value_ptr(glmmat), sizeof(double) * 16);
    }

    inline vsg::dmat4 glm2vsg(const glm::dmat4x4& glmmat)
    {
        vsg::dmat4 result;
        setdmat4(result, glmmat);
        return result;
    }

    inline vsg::dvec2 glm2vsg(const glm::dvec2& vec2)
    {
        return vsg::dvec2(vec2.x, vec2.y);
    }

    inline vsg::dvec3 glm2vsg(const glm::dvec3& vec3)
    {
        return vsg::dvec3(vec3.x, vec3.y, vec3.z);
    }

    inline bool isIdentity(const glm::dmat4x4& mat)
    {
        for (int c = 0; c < 4; ++c)
        {
            for (int r = 0; r < 4; ++r)
            {
                if (c == r)
                {
                    if (mat[c][r] != 1.0)
                        return false;
                }
                else
                {
                    if (mat[c][r] != 0.0)
                        return false;
                }
            }
        }
        return true;
    }

    template<class T>
    vsg::ref_ptr<T> read_cast(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {})
    {
        static const vsg::Paths libraryPaths{VSGCS_FULL_DATA_DIR};
        vsg::Path filePath = vsg::findFile(filename, options);
        if (filePath.empty())
        {
            filePath = vsg::findFile(filename, libraryPaths);
        }
        if (filePath.empty())
        {
            return {};
        }
        auto object = read(filePath, options);
        return vsg::ref_ptr<T>(dynamic_cast<T*>(object.get()));
    }
}
