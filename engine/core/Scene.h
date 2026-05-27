#pragma once

#include "Events.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "entt.hpp"

namespace OctalEngine
{
    struct Vec3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct Quaternion
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 1.0f;
    };

    struct Color
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;
    };

    struct Component
    {
    };

    struct TransformComponent : Component
    {
        Vec3 position{0.0f, 0.0f, 0.0f};
        Vec3 scale{1.0f, 1.0f, 1.0f};
        Quaternion rotation{};
    };

    struct CameraComponent : Component
    {
        float fov = 60.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        bool isOrthographic = false;
    };

    enum class PrimitiveType
    {
        Cube,
        Sphere,
        Plane,
        Capsule,
        Cylinder,
        Custom, // For future use when we implement mesh loading
    };

    struct MeshGeometry : Component
    {
        PrimitiveType primitive = PrimitiveType::Cube;
        // for future: AssetHandle mesh; // if primitive is Custom
    };

    struct MeshRendererComponent : Component
    {
        bool visible = true;
        bool castShadows = true;
        bool receiveShadows = true;
        int sortingOrder = 0;
        uint32_t renderLayer = 0;
    };

    enum class LightType
    {
        Directional,
        Point,
        Spot,
    };

    struct LightComponent : Component
    {
        float intensity = 1.0f;
        Color color{};
        LightType type = LightType::Directional;
        bool castShadows = false;
        float shadowBias = 0.005f;
        float shadowStrength = 1.0f;
        float shadowFarPlane = 100.0f;
    };

    class Object;
    class Scene;

    template <typename T>
    struct ComponentRequiresTransform : std::false_type
    {
    };

    template <>
    struct ComponentRequiresTransform<CameraComponent> : std::true_type
    {
    };

    template <>
    struct ComponentRequiresTransform<MeshRendererComponent> : std::true_type
    {
    };

    template <>
    struct ComponentRequiresTransform<LightComponent> : std::true_type
    {
    };

    struct SceneLoaded
    {
        Scene *scene = nullptr;
    };

    struct SceneUnloaded
    {
        Scene *scene = nullptr;
    };

    struct SceneActivated
    {
        Scene *scene = nullptr;
    };

    struct SceneDeactivated
    {
        Scene *scene = nullptr;
    };

    struct SceneUpdated
    {
        Scene *scene = nullptr;
        float dt = 0.0f;
    };

    class Object
    {
    public:
        Object() = default;
        virtual ~Object() = default;

        bool valid() const;
        std::uint32_t id() const;
        Scene *scene();
        const Scene *scene() const;

        void setName(std::string name);
        std::string_view name() const;

        template <typename T, typename... Args>
        T *addComponent(Args &&...args);

        template <typename T>
        T *getComponent();

        template <typename T>
        const T *getComponent() const;

        template <typename T>
        bool hasComponent() const;

        template <typename T>
        bool removeComponent();

        void setParent(const Object &parent);
        void clearParent();
        Object parent() const;
        std::vector<Object> children() const;

        TransformComponent worldTransform() const;
        Vec3 forward() const;

        bool operator==(const Object &other) const;
        bool operator!=(const Object &other) const;

    protected:
        Object(Scene *scene, std::uint32_t id);

    private:
        friend class Scene;

        Scene *owningScene = nullptr;
        std::uint32_t objectId = 0;
    };

    struct ObjectCreated
    {
        Scene *scene = nullptr;
        Object object;
    };

    struct ObjectDestroyed
    {
        Scene *scene = nullptr;
        std::uint32_t objectId = 0;
    };

    class Scene
    {
    public:
        explicit Scene(std::string name = {});
        virtual ~Scene();

        Scene(const Scene &) = delete;
        Scene &operator=(const Scene &) = delete;
        Scene(Scene &&) noexcept;
        Scene &operator=(Scene &&) noexcept;

        std::string_view name() const;
        void setName(std::string name);

        Object createObject(std::string name = {});
        void destroyObject(const Object &object);
        bool isValid(const Object &object) const;

        void setPrimaryCamera(const Object &object);
        Object primaryCamera() const;
        bool hasPrimaryCamera() const;

        void setParent(const Object &child, const Object &parent);
        void clearParent(const Object &child);
        Object parentOf(const Object &child) const;
        std::vector<Object> childrenOf(const Object &object) const;
        TransformComponent worldTransformOf(const Object &object) const;
        Vec3 forwardOf(const Object &object) const;

        template <typename T, typename... Args>
        T *addComponent(const Object &object, Args &&...args);

        template <typename T>
        T *getComponent(const Object &object);

        template <typename T>
        const T *getComponent(const Object &object) const;

        template <typename T>
        bool hasComponent(const Object &object) const;

        template <typename T>
        bool removeComponent(const Object &object);

        template <typename... Components, typename Callback>
        void each(Callback &&callback);

        template <typename... Components, typename Callback>
        void each(Callback &&callback) const;

        EventBus *events();
        const EventBus *events() const;

    protected:
        virtual void onLoad() {}
        virtual void onUnload() {}
        virtual void onUpdate(float) {}

    private:
        friend class Object;
        friend class SceneManager;

        struct Relationship;
        struct Name;

        entt::entity entityFor(const Object &object) const;
        Object objectFor(entt::entity entity);
        Object objectFor(entt::entity entity) const;
        void attachEvents(EventBus *events);
        void load();
        void unload();
        void update(float dt);
        void ensureMainThread() const;
        bool wouldCreateCycle(entt::entity child, entt::entity parent) const;
        void detachFromParent(entt::entity child);
        void destroyObjectRecursive(entt::entity entity);

        std::string sceneName;
        std::unique_ptr<entt::registry> registry;
        EventBus *sceneEvents = nullptr;
        entt::entity primaryCameraEntity = entt::null;
        std::thread::id owningThread;
    };

    class SceneManager
    {
    public:
        SceneManager();
        virtual ~SceneManager() = default;

        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;

        void setEventWorld(EventWorld *events);
        EventWorld *eventWorld();

        void load(std::unique_ptr<Scene> scene);
        void unload();
        void update(float dt);

        Scene *currentScene();
        const Scene *currentScene() const;
        bool hasScene() const;

    protected:
        virtual void onSceneChanged(Scene *) {}

    private:
        void ensureMainThread() const;
        void applyPendingScene();

        std::unique_ptr<Scene> activeScene;
        std::unique_ptr<Scene> pendingScene;
        bool unloadRequested = false;
        EventWorld *events = nullptr;
        std::thread::id owningThread;
    };

    template <typename T, typename... Args>
    T *Object::addComponent(Args &&...args)
    {
        return owningScene != nullptr ? owningScene->addComponent<T>(*this, std::forward<Args>(args)...) : nullptr;
    }

    template <typename T>
    T *Object::getComponent()
    {
        return owningScene != nullptr ? owningScene->getComponent<T>(*this) : nullptr;
    }

    template <typename T>
    const T *Object::getComponent() const
    {
        return owningScene != nullptr ? owningScene->getComponent<T>(*this) : nullptr;
    }

    template <typename T>
    bool Object::hasComponent() const
    {
        return owningScene != nullptr && owningScene->hasComponent<T>(*this);
    }

    template <typename T>
    bool Object::removeComponent()
    {
        return owningScene != nullptr && owningScene->removeComponent<T>(*this);
    }

    template <typename T, typename... Args>
    T *Scene::addComponent(const Object &object, Args &&...args)
    {
        ensureMainThread();

        if (!isValid(object))
        {
            return nullptr;
        }

        if constexpr (ComponentRequiresTransform<T>::value)
        {
            if (!hasComponent<TransformComponent>(object))
            {
                return nullptr;
            }
        }

        return &registry->emplace_or_replace<T>(entityFor(object), std::forward<Args>(args)...);
    }

    template <typename T>
    T *Scene::getComponent(const Object &object)
    {
        ensureMainThread();

        if (!isValid(object))
        {
            return nullptr;
        }

        return registry->try_get<T>(entityFor(object));
    }

    template <typename T>
    const T *Scene::getComponent(const Object &object) const
    {
        ensureMainThread();

        if (!isValid(object))
        {
            return nullptr;
        }

        return registry->try_get<T>(entityFor(object));
    }

    template <typename T>
    bool Scene::hasComponent(const Object &object) const
    {
        ensureMainThread();
        return isValid(object) && registry->all_of<T>(entityFor(object));
    }

    template <typename T>
    bool Scene::removeComponent(const Object &object)
    {
        ensureMainThread();

        if (!isValid(object) || !registry->all_of<T>(entityFor(object)))
        {
            return false;
        }

        registry->remove<T>(entityFor(object));

        if constexpr (std::is_same_v<T, CameraComponent>)
        {
            if (primaryCameraEntity == entityFor(object))
            {
                primaryCameraEntity = entt::null;
            }
        }

        return true;
    }

    template <typename... Components, typename Callback>
    void Scene::each(Callback &&callback)
    {
        ensureMainThread();

        auto view = registry->view<Components...>();
        for (const entt::entity entity : view)
        {
            std::apply(
                [&](auto &...components)
                {
                    callback(objectFor(entity), components...);
                },
                view.get(entity));
        }
    }

    template <typename... Components, typename Callback>
    void Scene::each(Callback &&callback) const
    {
        ensureMainThread();

        auto view = registry->view<Components...>();
        for (const entt::entity entity : view)
        {
            std::apply(
                [&](const auto &...components)
                {
                    callback(objectFor(entity), components...);
                },
                view.get(entity));
        }
    }
}
