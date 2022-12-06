#include "CesiumGltfBuilder.h"

#include <algorithm>

using namespace vsgCesium;
using namespace CesiumGltf;

namespace
{
    bool isGltfIdentity(const std::vector<double>& matrix)
    {
        if (matrix.size() != 16)
            return false;
        for (i = 0; i < 16; ++i)
        {
            if (i % 5 == 0)
            {
                if (matrix[i] != 1.0)
                    return false;
            }
            else
            {
                if (matrix[i] != 0.0)
                    return false;
            }
        }
        return true;
    }
}

vsg::ref_ptr<vsg::Group>
loadNode(const CesiumGltf::Node* node)
{
    const std::vector<double>& matrix = node.matrix;
    vsg::ref_ptr<vsg::Group> result;
    if (isGltfIdentity(matrix))
    {
        result = vsg::Group::create();
    }
    else
    {
        vsg::dmat4 transformMatrix;
        if (matrix.size() == 16)
        {
            std::copy(matrix.begin(), matrix.end(), transformMatrix.data());
        }
        else
        {
            vsg::dmat4 translation;
            if (node.translation.size() == 3)
            {
                translation = vsg::translate(node.translation[0], node.translation[1], node.translation[2]);
            }
            vsg::dquat rotation(0.0, 0.0, 0.0, 1.0);
            if (node.rotation.size() == 4)
            {
                rotation.x = node.rotation[1];
                rotation.y = node.rotation[2];
                rotation.z = node.rotation[3];
                rotation.w = node.rotation[0];
            }
            vsg::dmat4 scale;
            if (node.scale.size() == 3)
            {
                scale = vsg::scale(node.scale[0], node.scale[1], node.scale[2])
            }
            transformMatrix = translation * rotate(rotation) * scale;
        }
        result = vsg::<vsg::MatrixTransform>::create(transformMatrix);
    }
    int meshId = node.mesh;
    if (meshId >= 0 && meshId < model->meshes.size())
    {
        result->addChild(loadMesh(&model->meshes[meshId]));
    }
    for (int childNodeId : node.children)
    {
        if (childNodeId >= 0 && childNodeId < model->nodes.size())
        {
            result->addChild(loadNode(&model->nodes[childNodeId]));
        }
    }
    return result;
}

vsg::ref_ptr<vsg::Group> load()
{
    vsg::ref_ptr<vsg::Group> resultNode = vsg::Group::create();

    if (model->scene >= 0 && model->scene < model->scenes.size())
    {
        // Show the default scene
        const Scene& defaultScene = model->scenes[model->scene];
        for (int nodeId : defaultScene.nodes)
        {
            resultNode->addChild(loadNode(&model->nodes[nodeId]));
        }
    }
    else if (model->scenes.size() > 0)
    {
        // There's no default, so show the first scene
        const Scene& defaultScene = model->scenes[0];
        for (int nodeId : defaultScene.nodes)
        {
            resultNode->addChild(loadNode(&model->nodes[nodeId]));
        }
    }
    else if (model->nodes.size() > 0)
    {
        // No scenes at all, use the first node as the root node.
        resultNode = loadNode(&model->nodes[0]);
    }
    else if (model->meshes.size() > 0)
    {
        // No nodes either, show all the meshes.
        for (const Mesh& mesh : model->meshes)
        {
            resultNode->addChild(loadMesh(&mesh));
        }
    }
    return resultNode;
}
