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


#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/IAssetResponse.h>
#include <CesiumGltfReader/GltfReader.h>
#include <Cesium3DTilesSelection/GltfUtilities.h>

#include <filesystem>

using namespace vsgCs;

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

#if 0
CesiumAsync::Future<ReadGltfResult>
loadGltfNode(std::string uri, vsg::ref_ptr<RuntimeEnvironment> env)
{
    using namespace CesiumGltfReader;
    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;
    auto accessor = env-> getAssetAccessor();
    return accessor->get(getAsyncSystem(), uri, headers)
        .thenImmediately([&uri, accessor, env](std::shared_ptr<CesiumAsync::IAssetRequest>&& pRequest)
        {
            const CesiumAsync::IAssetResponse* pResponse = pRequest->response();
            if (pResponse == nullptr)
            {
                return std::make_shared<ParseGltfResult>("Request for " + uri + "failed.", pRequest);
            }
            if (pResponse->data().empty())
            {
                return std::make_shared<ParseGltfResult>("data for " + pRequest->url() + " is empty.",
                                                         pRequest);
            }
            GltfReader reader;
            auto gltfResult = reader.readGltf(pRequest->response()->data());
            return std::make_shared<ParseGltfResult>(std::move(gltfResult), pRequest);
        })
        .thenInWorkerThread([accessor, env](std::shared_ptr<ParseGltfResult>&& parseResult)
        {
            CesiumGltfReader::GltfReaderOptions gltfOptions;
            gltfOptions.ktx2TranscodeTargets = env->features.ktx2TranscodeTargets;
            CesiumAsync::HttpHeaders requestHeaders = parseResult->request->headers();
            std::string baseUrl = parseResult->request->url();

            return GltfReader::resolveExternalData(getAsyncSystem(),
                                                   baseUrl,
                                                   requestHeaders,
                                                   accessor,
                                                   gltfOptions,
                                                   
                std::move(parseResult->gltfResult));
        })
        .thenInWorkerThread([env](GltfReaderResult&& gltfResult)
        {
            for (auto& err : gltfResult.errors)
            {
                vsg::warn(err);
            }
            for (auto& warn : gltfResult.warnings)
            {
                vsg::warn(warn);
            }
            CreateModelOptions modelOptions{};
            ModelBuilder modelBuilder(env->genv, &*gltfResult.model, modelOptions);
            glm::dmat4 yUp(1.0);
            yUp = Cesium3DTilesSelection::GltfUtilities::applyGltfUpAxisTransform(*gltfResult.model, yUp);
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
#endif

CesiumAsync::Future<ReadGltfResult>
loadGltfNode(std::string uri, vsg::ref_ptr<RuntimeEnvironment> env)
{
    using namespace CesiumGltfReader;
    std::vector<CesiumAsync::IAssetAccessor::THeader> headers;
    auto accessor = env-> getAssetAccessor();
    return GltfReader::loadGltf(getAsyncSystem(), uri, headers, accessor)
        .thenInWorkerThread([env](GltfReaderResult&& gltfResult)
        {
            CreateModelOptions modelOptions{};
            ModelBuilder modelBuilder(env->genv, &*gltfResult.model, modelOptions);
            glm::dmat4 yUp(1.0);
            yUp = Cesium3DTilesSelection::GltfUtilities::applyGltfUpAxisTransform(*gltfResult.model, yUp);
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
    auto future = loadGltfNode(uriPath, RuntimeEnvironment::get());
    getAsyncSystem().dispatchMainThreadTasks();

    auto loadResult = future.wait();

    return loadResult.node;
}
