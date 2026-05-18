#pragma once

#include "../backend/IRendererBackend.h"
#include "RenderCommands.h"

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace OctalEngine::RendererInternal
{
    class RenderThread
    {
    public:
        RenderThread();
        ~RenderThread();

        RenderThread(const RenderThread&) = delete;
        RenderThread& operator=(const RenderThread&) = delete;

        bool start(std::unique_ptr<RendererBackend::IRendererBackend> backend,
                   const RendererBackend::BackendSettings& settings);
        void stop();

        void submit(CommandBuffer buffer, bool waitForCompletion);

    private:
        struct PendingFrame
        {
            std::uint64_t id = 0;
            CommandBuffer commands;
        };

        void run(RendererBackend::BackendSettings settings);

        std::mutex mutex;
        std::condition_variable frameAvailable;
        std::condition_variable frameCompleted;
        std::condition_variable backendInitialized;
        std::deque<PendingFrame> frames;
        std::unique_ptr<RendererBackend::IRendererBackend> backend;
        std::thread thread;
        std::uint64_t nextFrameId = 1;
        std::uint64_t completedFrameId = 0;
        bool stopping = false;
        bool started = false;
        bool backendReady = false;
        bool backendInitializationComplete = false;
    };
}
