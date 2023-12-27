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

#include "GltfLoader.h"

#include "ModelBuilder.h"
#include "OpThreadTaskProcessor.h"
#include "RuntimeEnvironment.h"
#include "runtimeSupport.h"
#include "Styling.h"


#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/IAssetResponse.h>
#include <CesiumGltfContent/GltfUtilities.h>

#include <filesystem>

using namespace vsgCs;

GltfLoader::GltfLoader(vsg::ref_ptr<RuntimeEnvironment> in_env)
    : env(in_env)
{
}

struct ParseGltfResult
{
    ParseGltfResult(const std::string& error, std::shared_ptr<CesiumAsync::IAssetRequest> in_request)
        : request(in_request)
    {
        gltfResult.errors.push_back(error);
    }
    ParseGltfResult(CesiumGltfReader::GltfReaderResult&& result,
        std::shared_ptr<CesiumAsync::IAssetRequest> in_request)
        : gltfResult(std::move(result)) ,request(in_request)
    {
    }

    ParseGltfResult()
    {
    }
    CesiumGltfReader::GltfReaderResult gltfResult;
    std::shared_ptr<CesiumAsync::IAssetRequest> request;
};

struct ReadGltfResult
{
    vsg::ref_ptr<vsg::Node> node;
    std::vector<std::string> errors;
};

CesiumAsync::Future<GltfLoader::ReadGltfResult> GltfLoader::loadGltfNode(std::string uri) const
{
    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;
    auto accessor = env-> getAssetAccessor();
    return reader.loadGltf(getAsyncSystem(), uri, headers, accessor)
        .thenInWorkerThread([this](CesiumGltfReader::GltfReaderResult&& gltfResult)
        {
            CreateModelOptions modelOptions{};
            ModelBuilder modelBuilder(env->genv, &*gltfResult.model, modelOptions);
            glm::dmat4 yUp(1.0);
            yUp = CesiumGltfContent::GltfUtilities::applyGltfUpAxisTransform(*gltfResult.model, yUp);
            auto modelNode = modelBuilder();
            if (isIdentity(yUp))
            {
                return ReadGltfResult{modelNode, {}};
            }
            auto transformNode = vsg::MatrixTransform::create(glm2vsg(yUp));
            transformNode->addChild(modelNode);
            return ReadGltfResult{transformNode, {}};
        });
}

vsg::ref_ptr<vsg::Object>
GltfLoader::read(const vsg::Path& path, vsg::ref_ptr<const vsg::Options> options) const
{
    using namespace CesiumGltfReader;
    auto realPath = vsg::findFile(path, options);
    if (realPath.empty())
    {
        vsg::fatal("Can't find file ", path);
    }
    // Really need an absolute path in order to make a URI
    std::filesystem::path p = realPath.string();
    auto absPath = std::filesystem::absolute(p);
    auto uriPath = "file://" + absPath.string();
    auto future = loadGltfNode(uriPath);
    getAsyncSystem().dispatchMainThreadTasks();

    auto loadResult = future.wait();

    return loadResult.node;
}
