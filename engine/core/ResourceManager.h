#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace OctalEngine
{
    class ResourceManager
    {
    public:
        using AssetHandle = uint64_t;
        enum class Status
        {
            Unknown = 0,
            Loading,
            Loaded,
            NotFound,
            Error
        };

        struct Asset
        {
            AssetHandle id = 0;
            std::string path;
            std::vector<char> data;
            Status status = Status::Unknown;
            std::string error;
        };

        using AssetPtr = std::shared_ptr<Asset>;
        using LoadCallback = std::function<void(AssetHandle)>;

        static void Initialize();
        static void Shutdown();

        // Request an asset to be loaded. Returns a handle immediately.
        // Optionally provide a callback invoked on completion (success or error).
        static AssetHandle LoadAsset(const std::string& path, LoadCallback onComplete = nullptr);

        // Query asset by handle or path. May return nullptr if not registered or not yet loaded.
        static AssetPtr GetAsset(AssetHandle handle);
        static AssetPtr GetAsset(const std::string& path);

        // Helpers
        static Status GetStatus(AssetHandle handle);

    private:
        ResourceManager() = default;

        struct Entry;

        static ResourceManager& instance();

        AssetHandle registerAsset(const std::string& path, LoadCallback onComplete);
        AssetPtr getByHandle(AssetHandle handle);

        std::mutex mutex;
        std::unordered_map<std::string, std::shared_ptr<Entry>> pathMap;
        std::unordered_map<AssetHandle, std::shared_ptr<Entry>> handleMap;
        std::atomic<AssetHandle> nextId{1};
    };
}
