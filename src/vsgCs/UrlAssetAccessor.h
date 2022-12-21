#pragma once

#include "Export.h"

#include "CesiumAsync/AsyncSystem.h"
#include "CesiumAsync/IAssetAccessor.h"
#include <cstddef>

namespace vsgCs
{
// Simple implementation of AssetAcessor that can make network and local requests
    
    class VSGCS_EXPORT UrlAssetAccessor
        : public CesiumAsync::IAssetAccessor {
    public:
        UrlAssetAccessor();
        ~UrlAssetAccessor() override;

        virtual CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>>
            get(const CesiumAsync::AsyncSystem& asyncSystem,
                const std::string& url,
                const std::vector<CesiumAsync::IAssetAccessor::THeader>& headers)
            override;

        virtual CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>>
            request(
                const CesiumAsync::AsyncSystem& asyncSystem,
                const std::string& verb,
                const std::string& url,
                const std::vector<CesiumAsync::IAssetAccessor::THeader>& headers,
                const gsl::span<const std::byte>& contentPayload) override;

        virtual void tick() noexcept override;

    private:
        std::string _userAgent;
    };
}
