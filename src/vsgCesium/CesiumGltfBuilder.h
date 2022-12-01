#pragma once

#include <vsg/all.h>

#include <CesiumGltf/Model.h>

// Build a VSG scenegraph from a Cesium Gltf Model object.

namespace vsgCesium
{
    class CesiumGltfBuilder
    {
    public:
        CesiumGltfBuilder(CesiumGltf::Model* model);
        vsg::ref_ptr<vsg::Group> load();
    private:
        CesiumGltf::Model* _model;
    };

    
    inline void setdmat4(vsg::dmat4& vmat, const glm::dmat4x4& glmmat)
    {
        std::memcpy(vmat.data(), glm::value_ptr(glmmat), sizeof(double) * 16);
    }

    inline vsg::dmat4 glm2vsg(const glm::dmat4x4& glmmat)
    {
        vsg::dmat4 result;
        setdmat4(result, glmmat);
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

}
