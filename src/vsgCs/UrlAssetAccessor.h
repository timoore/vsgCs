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

#pragma once

#include "vsgCs/Export.h"

#include "CesiumAsync/AsyncSystem.h"
#include "CesiumAsync/IAssetAccessor.h"

#include <curl/curl.h>

#include <mutex>
#include <forward_list>
#include <vector>
#include <memory>

#include <cstddef>

namespace vsgCs
{
    // A cache that permits reuse of CURL handles. This is extremely important for performance
    // because libcurl will keep existing connections open if a curl handle is not destroyed
    // ("cleaned up"). Our performance went from 180ms per get request to 60ms with this change.
    
    struct CurlCache
    {
        struct CurlObject
        {
            CurlObject()
                : curl(nullptr)
            {
            }
            explicit CurlObject(CURL* in_curl)
                : curl(in_curl)
            {
            }
            CURL* curl;
            char errbuf[CURL_ERROR_SIZE];
        };
        std::mutex cacheMutex;
        std::forward_list<std::unique_ptr<CurlObject>> cache;
        std::unique_ptr<CurlObject> get()
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            if (!cache.empty())
            {
                std::unique_ptr<CurlObject> result = std::move(cache.front());
                cache.pop_front();
                return result;
            }
            CURL *curl = curl_easy_init();
            auto result = std::make_unique<CurlObject>(curl);
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, result->errbuf);
            return result;
        }
        void release(std::unique_ptr<CurlObject> curlObject)
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            cache.emplace_front(std::move(curlObject));
        }
    };

    // Simple implementation of AssetAcessor that can make network and local requests
    class VSGCS_EXPORT UrlAssetAccessor
        : public CesiumAsync::IAssetAccessor {
    public:
        UrlAssetAccessor();
        ~UrlAssetAccessor() override;

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>>
            get(const CesiumAsync::AsyncSystem& asyncSystem,
                const std::string& url,
                const std::vector<CesiumAsync::IAssetAccessor::THeader>& headers)
            override;

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>>
            request(
                const CesiumAsync::AsyncSystem& asyncSystem,
                const std::string& verb,
                const std::string& url,
                const std::vector<CesiumAsync::IAssetAccessor::THeader>& headers,
                const std::span<const std::byte>& contentPayload) override;

        void tick() noexcept override;
        CurlCache curlCache;
        std::string userAgent;
    private:
        curl_slist* setCommonOptions(CURL* curl,
                                     const std::string& url,
                                     const CesiumAsync::HttpHeaders& headers);
        std::vector<std::string> _cesiumHeaders;
    };

    // RAII wrapper for the CurlCache.

    class CurlHandle
    {
    public:
        explicit CurlHandle(UrlAssetAccessor* in_accessor)
            : _accessor(in_accessor)

        {
            _curl = _accessor->curlCache.get();
        }

        ~CurlHandle()
        {
            _accessor->curlCache.release(std::move(_curl));
        }

        CURL* operator()() const
        {
            return _curl->curl;
        }
        const char *getErrBuf() const {
          return _curl->errbuf;
        }
    private:
        UrlAssetAccessor* _accessor;
        std::unique_ptr<CurlCache::CurlObject> _curl;
    };
}
