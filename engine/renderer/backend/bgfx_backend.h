#pragma once

#include "IRendererBackend.h"

#include <cstdint>
#include <memory>

namespace OctalEngine::RendererBackend
{
    class BgfxBackend final : public IRendererBackend
    {
    public:
        BgfxBackend();
        ~BgfxBackend() override;

        BgfxBackend(const BgfxBackend&) = delete;
        BgfxBackend& operator=(const BgfxBackend&) = delete;

        bool initialize(const BackendSettings& settings) override;
        void shutdown() override;
        void execute(const RendererInternal::CommandBuffer& commands) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}
