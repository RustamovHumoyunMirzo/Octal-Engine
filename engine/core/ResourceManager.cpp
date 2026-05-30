#include "ResourceManager.h"
#include "../jobs/JobSystem.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace OctalEngine
{
    struct ResourceManager::Entry
    {
        AssetPtr asset = std::make_shared<Asset>();
        std::mutex callbackMutex;
        std::vector<LoadCallback> callbacks;
    };

    ResourceManager& ResourceManager::instance()
    {
        static ResourceManager mgr;
        return mgr;
    }

    void ResourceManager::Initialize()
    {
        // Ensure the job system is running so dispatching load jobs doesn't block
        JobSystem::initialize();
    }

    void ResourceManager::Shutdown()
    {
        // Wait for pending loads to finish, then stop workers.
        JobSystem::wait();
        JobSystem::shutdown();

        // Clear internal caches
        ResourceManager& mgr = instance();
        std::scoped_lock lock(mgr.mutex);
        mgr.pathMap.clear();
        mgr.handleMap.clear();
        mgr.nextId = 1;
    }

    ResourceManager::AssetHandle ResourceManager::LoadAsset(const std::string& path, LoadCallback onComplete)
    {
        return instance().registerAsset(path, onComplete);
    }

    ResourceManager::AssetPtr ResourceManager::GetAsset(AssetHandle handle)
    {
        return instance().getByHandle(handle);
    }

    ResourceManager::AssetPtr ResourceManager::GetAsset(const std::string& path)
    {
        std::scoped_lock lock(instance().mutex);
        auto it = instance().pathMap.find(path);
        if (it == instance().pathMap.end())
        {
            return nullptr;
        }
        return it->second->asset;
    }

    ResourceManager::Status ResourceManager::GetStatus(AssetHandle handle)
    {
        auto a = GetAsset(handle);
        if (!a)
            return Status::Unknown;
        return a->status;
    }

    ResourceManager::AssetHandle ResourceManager::registerAsset(const std::string& path, LoadCallback onComplete)
    {
        std::shared_ptr<Entry> entry;

        {
            std::scoped_lock lock(mutex);
            auto it = pathMap.find(path);
            if (it != pathMap.end())
            {
                entry = it->second;
            }
            else
            {
                entry = std::make_shared<Entry>();
                AssetPtr asset = entry->asset;
                asset->id = nextId++;
                asset->path = path;
                asset->status = Status::Loading;

                pathMap.emplace(path, entry);
                handleMap.emplace(asset->id, entry);

                // Dispatch a job to load the file asynchronously
                AssetHandle id = asset->id;

                JobSystem::dispatch([entry, path, id]() {
                    try
                    {
                        if (!fs::exists(path))
                        {
                            entry->asset->status = Status::NotFound;
                            entry->asset->error = "file not found";
                        }
                        else
                        {
                            std::ifstream in(path, std::ios::binary);
                            if (!in)
                            {
                                entry->asset->status = Status::Error;
                                entry->asset->error = "failed to open file";
                            }
                            else
                            {
                                in.seekg(0, std::ios::end);
                                std::streamsize size = in.tellg();
                                in.seekg(0, std::ios::beg);

                                entry->asset->data.resize(static_cast<size_t>(size));
                                if (size > 0)
                                {
                                    if (!in.read(entry->asset->data.data(), size))
                                    {
                                        entry->asset->status = Status::Error;
                                        entry->asset->error = "failed to read file";
                                    }
                                    else
                                    {
                                        entry->asset->status = Status::Loaded;
                                    }
                                }
                                else
                                {
                                    // empty file
                                    entry->asset->status = Status::Loaded;
                                }
                            }
                        }
                    }
                    catch (const fs::filesystem_error& e)
                    {
                        entry->asset->status = Status::Error;
                        entry->asset->error = e.what();
                    }

                    // Collect callbacks to call outside locks
                    std::vector<LoadCallback> callbacks;
                    {
                        std::scoped_lock cbLock(entry->callbackMutex);
                        callbacks.swap(entry->callbacks);
                    }

                    for (auto &cb : callbacks)
                    {
                        try
                        {
                            if (cb)
                                cb(id);
                        }
                        catch (...) {}
                    }
                });
            }
        }

        // attach callback if provided
        if (onComplete)
        {
            std::scoped_lock cbLock(entry->callbackMutex);
            entry->callbacks.push_back(onComplete);
        }

        return entry->asset->id;
    }

    ResourceManager::AssetPtr ResourceManager::getByHandle(AssetHandle handle)
    {
        std::scoped_lock lock(mutex);
        auto it = handleMap.find(handle);
        if (it == handleMap.end())
            return nullptr;
        return it->second->asset;
    }

}
