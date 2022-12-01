#include "CesiumGltfBuilder.h"

vsg::ref_ptr<vsg::Group>
loadNode(const glm::dmat4& transform, CreateModelOptions& options, const CesiumGltf::Node*pNode)
{
    
}

vsg::ref_ptr<vsg::Group> load(CreateModelOptions& options)
{
    vsg::ref_ptr<vsg::Group> resultNode = vsg::Group::create();

    if (model->scene >= 0 && model->scene < model->scenes.size())
    {
        // Show the default scene
        const Scene& defaultScene = model->scenes[model->scene];
        for (int nodeId : defaultScene.nodes)
        {
            resultNode->addChild(loadNode(options, &model->nodes[nodeId]));
        }
    }
    else if (model->scenes.size() > 0)
    {
        // There's no default, so show the first scene
        const Scene& defaultScene = model->scenes[0];
        for (int nodeId : defaultScene.nodes)
        {
            resultNode->addChild(loadNode(options, &model->nodes[nodeId]));
        }
    }
    else if (model->nodes.size() > 0)
    {
        // No scenes at all, use the first node as the root node.
        resultNode = loadNode(nodeOptions, &model->nodes[0]);
    }
    else if (model->meshes.size() > 0)
    {
        // No nodes either, show all the meshes.
        for (const Mesh& mesh : model->meshes)
        {
            resultNode->addChild(loadMesh(options, &mesh));
        }
    }
    return resultNode;
}
