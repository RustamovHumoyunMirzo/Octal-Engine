#include "JobSystem.h"

#include <algorithm>

namespace OctalEngine
{
    JobSystem& JobSystem::instance()
    {
        static JobSystem system;
        return system;
    }

    void JobSystem::initialize(std::size_t workerCount)
    {
        instance().start(workerCount);
    }

    void JobSystem::shutdown()
    {
        instance().stop();
    }

    void JobSystem::dispatch(JobFunction job)
    {
        JobSystem& system = instance();
        system.start(0);
        system.enqueue(std::move(job));
    }

    void JobSystem::wait()
    {
        instance().waitForIdle();
    }

    void JobSystem::start(std::size_t workerCount)
    {
        std::scoped_lock lock(mutex);

        if (!workers.empty())
        {
            return;
        }

        stopping = false;

        const std::size_t hardwareThreads = std::max(1u, std::thread::hardware_concurrency());
        const std::size_t count = workerCount == 0 ? std::max<std::size_t>(1, hardwareThreads - 1) : workerCount;

        workers.reserve(count);
        for (std::size_t i = 0; i < count; ++i)
        {
            workers.emplace_back([this]() {
                workerLoop();
            });
        }
    }

    void JobSystem::stop()
    {
        {
            std::scoped_lock lock(mutex);
            stopping = true;
        }

        workAvailable.notify_all();

        for (std::thread& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }

        workers.clear();
    }

    void JobSystem::enqueue(JobFunction job)
    {
        if (!job)
        {
            return;
        }

        {
            std::scoped_lock lock(mutex);

            jobs.push(std::move(job));
        }

        workAvailable.notify_one();
    }

    void JobSystem::waitForIdle()
    {
        std::unique_lock lock(mutex);
        idle.wait(lock, [this]() {
            return jobs.empty() && activeJobs.load() == 0;
        });
    }

    void JobSystem::workerLoop()
    {
        while (true)
        {
            JobFunction job;

            {
                std::unique_lock lock(mutex);
                workAvailable.wait(lock, [this]() {
                    return stopping || !jobs.empty();
                });

                if (stopping && jobs.empty())
                {
                    return;
                }

                job = std::move(jobs.front());
                jobs.pop();
                ++activeJobs;
            }

            job();

            {
                std::scoped_lock lock(mutex);
                --activeJobs;
            }

            idle.notify_all();
        }
    }
}
