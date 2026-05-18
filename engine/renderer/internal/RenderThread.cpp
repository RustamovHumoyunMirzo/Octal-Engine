#include "RenderThread.h"

namespace OctalEngine::RendererInternal
{
    RenderThread::RenderThread() = default;

    RenderThread::~RenderThread()
    {
        stop();
    }

    bool RenderThread::start(std::unique_ptr<RendererBackend::IRendererBackend> renderBackend,
                             const RendererBackend::BackendSettings& settings)
    {
        if (started)
        {
            return backendReady;
        }

        backend = std::move(renderBackend);
        stopping = false;
        started = true;

        thread = std::thread([this, settings]() {
            run(settings);
        });

        bool ready = false;

        {
            std::unique_lock lock(mutex);
            backendInitialized.wait(lock, [this]() {
                return backendInitializationComplete;
            });

            ready = backendReady;
        }

        if (!ready)
        {
            stop();
        }

        return ready;
    }

    void RenderThread::stop()
    {
        {
            std::scoped_lock lock(mutex);
            stopping = true;
        }

        frameAvailable.notify_all();

        if (thread.joinable())
        {
            thread.join();
        }

        started = false;
        backendReady = false;
        backendInitializationComplete = false;
        backend.reset();
    }

    void RenderThread::submit(CommandBuffer buffer, bool waitForCompletion)
    {
        std::uint64_t frameId = 0;

        {
            std::scoped_lock lock(mutex);

            if (!started || !backendReady)
            {
                return;
            }

            frameId = nextFrameId++;
            frames.push_back(PendingFrame{frameId, std::move(buffer)});
        }

        frameAvailable.notify_one();

        if (!waitForCompletion)
        {
            return;
        }

        std::unique_lock lock(mutex);
        frameCompleted.wait(lock, [this, frameId]() {
            return completedFrameId >= frameId;
        });
    }

    void RenderThread::run(RendererBackend::BackendSettings settings)
    {
        bool initialized = true;

        if (backend != nullptr)
        {
            initialized = backend->initialize(settings);
        }

        {
            std::scoped_lock lock(mutex);
            backendReady = initialized;
            backendInitializationComplete = true;
        }

        backendInitialized.notify_all();

        while (true)
        {
            PendingFrame frame;

            {
                std::unique_lock lock(mutex);
                frameAvailable.wait(lock, [this]() {
                    return stopping || !frames.empty();
                });

                if (stopping && frames.empty())
                {
                    break;
                }

                frame = std::move(frames.front());
                frames.pop_front();
            }

            if (backend != nullptr)
            {
                backend->execute(frame.commands);
            }

            {
                std::scoped_lock lock(mutex);
                completedFrameId = frame.id;
            }

            frameCompleted.notify_all();
        }

        if (backend != nullptr)
        {
            backend->shutdown();
        }
    }
}
