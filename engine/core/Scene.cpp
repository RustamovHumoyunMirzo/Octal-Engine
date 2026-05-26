#include "Scene.h"

#include <algorithm>
#include <stdexcept>

namespace OctalEngine
{
    struct Scene::Relationship
    {
        entt::entity parent = entt::null;
        std::vector<entt::entity> children;
    };

    struct Scene::Name
    {
        std::string value;
    };

    namespace
    {
        Vec3 multiply(const Vec3& a, const Vec3& b)
        {
            return {a.x * b.x, a.y * b.y, a.z * b.z};
        }

        Vec3 add(const Vec3& a, const Vec3& b)
        {
            return {a.x + b.x, a.y + b.y, a.z + b.z};
        }

        Quaternion multiply(const Quaternion& a, const Quaternion& b)
        {
            return {
                a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
                a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
                a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
                a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z};
        }

        Quaternion conjugate(const Quaternion& q)
        {
            return {-q.x, -q.y, -q.z, q.w};
        }

        Vec3 rotate(const Quaternion& q, const Vec3& v)
        {
            const Quaternion vector{v.x, v.y, v.z, 0.0f};
            const Quaternion rotated = multiply(multiply(q, vector), conjugate(q));
            return {rotated.x, rotated.y, rotated.z};
        }

        TransformComponent compose(const TransformComponent& parent, const TransformComponent& local)
        {
            TransformComponent result;
            result.scale = multiply(parent.scale, local.scale);
            result.rotation = multiply(parent.rotation, local.rotation);
            result.position = add(parent.position, rotate(parent.rotation, multiply(parent.scale, local.position)));
            return result;
        }
    }

    Object::Object(Scene* scene, std::uint32_t id)
        : owningScene(scene), objectId(id)
    {
    }

    bool Object::valid() const
    {
        return owningScene != nullptr && owningScene->isValid(*this);
    }

    std::uint32_t Object::id() const
    {
        return objectId;
    }

    Scene* Object::scene()
    {
        return owningScene;
    }

    const Scene* Object::scene() const
    {
        return owningScene;
    }

    void Object::setName(std::string name)
    {
        if (valid())
        {
            owningScene->registry->emplace_or_replace<Scene::Name>(
                owningScene->entityFor(*this), Scene::Name{std::move(name)});
        }
    }

    std::string_view Object::name() const
    {
        if (!valid())
        {
            return {};
        }

        const auto* nameComponent = owningScene->registry->try_get<Scene::Name>(owningScene->entityFor(*this));
        return nameComponent != nullptr ? std::string_view{nameComponent->value} : std::string_view{};
    }

    void Object::setParent(const Object& parent)
    {
        if (owningScene != nullptr)
        {
            owningScene->setParent(*this, parent);
        }
    }

    void Object::clearParent()
    {
        if (owningScene != nullptr)
        {
            owningScene->clearParent(*this);
        }
    }

    Object Object::parent() const
    {
        return owningScene != nullptr ? owningScene->parentOf(*this) : Object{};
    }

    std::vector<Object> Object::children() const
    {
        return owningScene != nullptr ? owningScene->childrenOf(*this) : std::vector<Object>{};
    }

    TransformComponent Object::worldTransform() const
    {
        return owningScene != nullptr ? owningScene->worldTransformOf(*this) : TransformComponent{};
    }

    Vec3 Object::forward() const
    {
        return owningScene != nullptr ? owningScene->forwardOf(*this) : Vec3{0.0f, 0.0f, -1.0f};
    }

    bool Object::operator==(const Object& other) const
    {
        return owningScene == other.owningScene && objectId == other.objectId;
    }

    bool Object::operator!=(const Object& other) const
    {
        return !(*this == other);
    }

    Scene::Scene(std::string name)
        : sceneName(std::move(name)),
          registry(std::make_unique<entt::registry>()),
          owningThread(std::this_thread::get_id())
    {
    }

    Scene::~Scene() = default;

    Scene::Scene(Scene&& other) noexcept = default;
    Scene& Scene::operator=(Scene&& other) noexcept = default;

    std::string_view Scene::name() const
    {
        return sceneName;
    }

    void Scene::setName(std::string name)
    {
        sceneName = std::move(name);
    }

    Object Scene::createObject(std::string name)
    {
        ensureMainThread();

        const entt::entity entity = registry->create();
        registry->emplace<Relationship>(entity);

        Object object = objectFor(entity);
        if (!name.empty())
        {
            object.setName(std::move(name));
        }

        if (sceneEvents != nullptr)
        {
            sceneEvents->emit<ObjectCreated>({this, object});
        }

        return object;
    }

    void Scene::destroyObject(const Object& object)
    {
        ensureMainThread();

        if (!isValid(object))
        {
            return;
        }

        destroyObjectRecursive(entityFor(object));
    }

    bool Scene::isValid(const Object& object) const
    {
        if (object.owningScene != this || registry == nullptr)
        {
            return false;
        }

        const entt::entity entity = entityFor(object);
        return entity != entt::null && registry->valid(entity);
    }

    void Scene::setPrimaryCamera(const Object& object)
    {
        ensureMainThread();

        if (isValid(object) && hasComponent<CameraComponent>(object))
        {
            primaryCameraEntity = entityFor(object);
        }
    }

    Object Scene::primaryCamera() const
    {
        return primaryCameraEntity != entt::null && registry->valid(primaryCameraEntity)
            ? objectFor(primaryCameraEntity)
            : Object{};
    }

    bool Scene::hasPrimaryCamera() const
    {
        return primaryCamera().valid();
    }

    void Scene::setParent(const Object& child, const Object& parent)
    {
        ensureMainThread();

        if (!isValid(child) || !isValid(parent) || child == parent)
        {
            return;
        }

        const entt::entity childEntity = entityFor(child);
        const entt::entity parentEntity = entityFor(parent);

        if (wouldCreateCycle(childEntity, parentEntity))
        {
            return;
        }

        detachFromParent(childEntity);

        auto& childRelationship = registry->get_or_emplace<Relationship>(childEntity);
        auto& parentRelationship = registry->get_or_emplace<Relationship>(parentEntity);

        childRelationship.parent = parentEntity;
        parentRelationship.children.push_back(childEntity);
    }

    void Scene::clearParent(const Object& child)
    {
        ensureMainThread();

        if (isValid(child))
        {
            detachFromParent(entityFor(child));
        }
    }

    Object Scene::parentOf(const Object& child) const
    {
        ensureMainThread();

        if (!isValid(child))
        {
            return {};
        }

        const auto* relationship = registry->try_get<Relationship>(entityFor(child));
        if (relationship == nullptr || relationship->parent == entt::null || !registry->valid(relationship->parent))
        {
            return {};
        }

        return objectFor(relationship->parent);
    }

    std::vector<Object> Scene::childrenOf(const Object& object) const
    {
        ensureMainThread();

        std::vector<Object> children;
        if (!isValid(object))
        {
            return children;
        }

        const auto* relationship = registry->try_get<Relationship>(entityFor(object));
        if (relationship == nullptr)
        {
            return children;
        }

        children.reserve(relationship->children.size());
        for (const entt::entity child : relationship->children)
        {
            if (registry->valid(child))
            {
                children.push_back(objectFor(child));
            }
        }

        return children;
    }

    TransformComponent Scene::worldTransformOf(const Object& object) const
    {
        ensureMainThread();

        if (!isValid(object))
        {
            return {};
        }

        const entt::entity entity = entityFor(object);
        const auto* local = registry->try_get<TransformComponent>(entity);
        TransformComponent world = local != nullptr ? *local : TransformComponent{};

        const auto* relationship = registry->try_get<Relationship>(entity);
        if (relationship == nullptr || relationship->parent == entt::null || !registry->valid(relationship->parent))
        {
            return world;
        }

        return compose(worldTransformOf(objectFor(relationship->parent)), world);
    }

    Vec3 Scene::forwardOf(const Object& object) const
    {
        return rotate(worldTransformOf(object).rotation, {0.0f, 0.0f, -1.0f});
    }

    EventBus* Scene::events()
    {
        return sceneEvents;
    }

    const EventBus* Scene::events() const
    {
        return sceneEvents;
    }

    entt::entity Scene::entityFor(const Object& object) const
    {
        return static_cast<entt::entity>(object.objectId);
    }

    Object Scene::objectFor(entt::entity entity)
    {
        return Object(this, static_cast<std::uint32_t>(entity));
    }

    Object Scene::objectFor(entt::entity entity) const
    {
        return Object(const_cast<Scene*>(this), static_cast<std::uint32_t>(entity));
    }

    void Scene::attachEvents(EventBus* events)
    {
        sceneEvents = events;
    }

    void Scene::load()
    {
        ensureMainThread();
        onLoad();

        if (sceneEvents != nullptr)
        {
            sceneEvents->emit<SceneLoaded>({this});
            sceneEvents->emit<SceneActivated>({this});
        }
    }

    void Scene::unload()
    {
        ensureMainThread();

        if (sceneEvents != nullptr)
        {
            sceneEvents->emit<SceneDeactivated>({this});
        }

        onUnload();

        if (sceneEvents != nullptr)
        {
            sceneEvents->emit<SceneUnloaded>({this});
        }
    }

    void Scene::update(float dt)
    {
        ensureMainThread();
        onUpdate(dt);

        if (sceneEvents != nullptr)
        {
            sceneEvents->emit<SceneUpdated>({this, dt});
        }
    }

    void Scene::ensureMainThread() const
    {
        if (owningThread != std::this_thread::get_id())
        {
            throw std::runtime_error("Scene accessed from a thread other than its owning game-loop thread.");
        }
    }

    bool Scene::wouldCreateCycle(entt::entity child, entt::entity parent) const
    {
        entt::entity cursor = parent;

        while (cursor != entt::null && registry->valid(cursor))
        {
            if (cursor == child)
            {
                return true;
            }

            const auto* relationship = registry->try_get<Relationship>(cursor);
            cursor = relationship != nullptr ? relationship->parent : entt::null;
        }

        return false;
    }

    void Scene::detachFromParent(entt::entity child)
    {
        auto& childRelationship = registry->get_or_emplace<Relationship>(child);

        if (childRelationship.parent == entt::null || !registry->valid(childRelationship.parent))
        {
            childRelationship.parent = entt::null;
            return;
        }

        auto* parentRelationship = registry->try_get<Relationship>(childRelationship.parent);
        if (parentRelationship != nullptr)
        {
            auto& siblings = parentRelationship->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), child), siblings.end());
        }

        childRelationship.parent = entt::null;
    }

    void Scene::destroyObjectRecursive(entt::entity entity)
    {
        if (!registry->valid(entity))
        {
            return;
        }

        const std::uint32_t id = static_cast<std::uint32_t>(entity);

        auto* relationship = registry->try_get<Relationship>(entity);
        if (relationship != nullptr)
        {
            const auto children = relationship->children;
            for (const entt::entity child : children)
            {
                destroyObjectRecursive(child);
            }
        }

        detachFromParent(entity);

        if (primaryCameraEntity == entity)
        {
            primaryCameraEntity = entt::null;
        }

        registry->destroy(entity);

        if (sceneEvents != nullptr)
        {
            sceneEvents->emit<ObjectDestroyed>({this, id});
        }
    }

    SceneManager::SceneManager()
        : owningThread(std::this_thread::get_id())
    {
    }

    void SceneManager::setEventWorld(EventWorld* eventWorld)
    {
        ensureMainThread();
        events = eventWorld;

        if (activeScene != nullptr)
        {
            activeScene->attachEvents(events != nullptr ? &events->scene() : nullptr);
        }
    }

    EventWorld* SceneManager::eventWorld()
    {
        return events;
    }

    void SceneManager::load(std::unique_ptr<Scene> scene)
    {
        ensureMainThread();
        pendingScene = std::move(scene);
        unloadRequested = pendingScene == nullptr;
    }

    void SceneManager::unload()
    {
        ensureMainThread();
        pendingScene.reset();
        unloadRequested = true;
    }

    void SceneManager::update(float dt)
    {
        ensureMainThread();
        applyPendingScene();

        if (activeScene != nullptr)
        {
            activeScene->update(dt);
        }
    }

    Scene* SceneManager::currentScene()
    {
        return activeScene.get();
    }

    const Scene* SceneManager::currentScene() const
    {
        return activeScene.get();
    }

    bool SceneManager::hasScene() const
    {
        return activeScene != nullptr;
    }

    void SceneManager::ensureMainThread() const
    {
        if (owningThread != std::this_thread::get_id())
        {
            throw std::runtime_error("SceneManager accessed from a thread other than its owning game-loop thread.");
        }
    }

    void SceneManager::applyPendingScene()
    {
        if (pendingScene == nullptr && !unloadRequested)
        {
            return;
        }

        if (activeScene != nullptr)
        {
            activeScene->unload();
        }

        activeScene = std::move(pendingScene);
        unloadRequested = false;

        if (activeScene != nullptr)
        {
            activeScene->owningThread = owningThread;
            activeScene->attachEvents(events != nullptr ? &events->scene() : nullptr);
            activeScene->load();
        }

        onSceneChanged(activeScene.get());
    }
}
