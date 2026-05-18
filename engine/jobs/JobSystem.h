#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace OctalEngine
{
    class JobSystem
    {
    public:
        using JobFunction = std::function<void()>;

        static void initialize(std::size_t workerCount = 0);
        static void shutdown();

        static void dispatch(JobFunction job);

        template <typename Function>
        static void dispatchParallel(std::size_t count, Function&& function)
        {
            if (count == 0)
            {
                return;
            }

            for (std::size_t index = 0; index < count; ++index)
            {
                dispatch([index, function]() mutable {
                    function(index);
                });
            }
        }

        static void wait();

    private:
        JobSystem() = default;

        static JobSystem& instance();

        void start(std::size_t workerCount);
        void stop();
        void enqueue(JobFunction job);
        void waitForIdle();
        void workerLoop();

        std::mutex mutex;
        std::condition_variable workAvailable;
        std::condition_variable idle;
        std::queue<JobFunction> jobs;
        std::vector<std::thread> workers;
        std::atomic<std::size_t> activeJobs{0};
        bool stopping = false;
    };
}
