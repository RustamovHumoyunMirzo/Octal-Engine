#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace OctalEngine
{
    struct Update
    {
        float dt = 0.0f;
    };

    class Subscription
    {
    public:
        Subscription() = default;

        explicit Subscription(std::function<void()> unsubscribe)
            : unsubscribe(std::move(unsubscribe))
        {
        }

        ~Subscription()
        {
            reset();
        }

        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;

        Subscription(Subscription&& other) noexcept
            : unsubscribe(std::move(other.unsubscribe))
        {
            other.unsubscribe = nullptr;
        }

        Subscription& operator=(Subscription&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                unsubscribe = std::move(other.unsubscribe);
                other.unsubscribe = nullptr;
            }

            return *this;
        }

        void reset()
        {
            if (unsubscribe != nullptr)
            {
                unsubscribe();
                unsubscribe = nullptr;
            }
        }

        bool active() const
        {
            return unsubscribe != nullptr;
        }

    private:
        std::function<void()> unsubscribe;
    };

    class EventBus
    {
    public:
        EventBus()
            : state(std::make_shared<State>())
        {
        }

        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;

        template <typename Event>
        Subscription subscribe(std::function<void(const Event&)> callback)
        {
            const std::type_index type = std::type_index(typeid(Event));
            const std::size_t id = state->nextId++;

            Handler handler;
            handler.id = id;
            handler.callback = [callback = std::move(callback)](const void* event) {
                callback(*static_cast<const Event*>(event));
            };

            state->handlers[type].push_back(std::move(handler));

            std::weak_ptr<State> weakState = state;

            return Subscription([weakState, type, id]() {
                if (auto locked = weakState.lock())
                {
                    EventBus::unsubscribe(*locked, type, id);
                }
            });
        }

        template <typename Event, typename Callback>
        Subscription subscribe(Callback&& callback)
        {
            return subscribe<Event>(std::function<void(const Event&)>(std::forward<Callback>(callback)));
        }

        template <typename Event>
        void once(std::function<void(const Event&)> callback)
        {
            const std::type_index type = std::type_index(typeid(Event));
            const std::size_t id = state->nextId++;

            Handler handler;
            handler.id = id;
            handler.once = true;
            handler.callback = [callback = std::move(callback)](const void* event) {
                callback(*static_cast<const Event*>(event));
            };

            state->handlers[type].push_back(std::move(handler));
        }

        template <typename Event, typename Callback>
        void once(Callback&& callback)
        {
            once<Event>(std::function<void(const Event&)>(std::forward<Callback>(callback)));
        }

        template <typename Event>
        void emit(const Event& event)
        {
            const std::type_index type = std::type_index(typeid(Event));
            auto found = state->handlers.find(type);

            if (found == state->handlers.end())
            {
                return;
            }

            state->emitting++;

            for (auto& handler : found->second)
            {
                if (!handler.active)
                {
                    continue;
                }

                handler.callback(&event);

                if (handler.once)
                {
                    handler.active = false;
                }
            }

            state->emitting--;
            compactHandlers(*state, type);
        }

        template <typename Event>
        void emitDeferred(Event event)
        {
            state->deferred.push_back([weakState = std::weak_ptr<State>(state), event = std::move(event)]() {
                if (auto locked = weakState.lock())
                {
                    EventBus bus(locked);
                    bus.emit<Event>(event);
                }
            });
        }

        void flush()
        {
            auto pending = std::move(state->deferred);
            state->deferred.clear();

            for (auto& event : pending)
            {
                event();
            }
        }

    private:
        struct Handler
        {
            std::size_t id = 0;
            bool active = true;
            bool once = false;
            std::function<void(const void*)> callback;
        };

        struct State
        {
            std::size_t nextId = 1;
            std::size_t emitting = 0;
            std::unordered_map<std::type_index, std::vector<Handler>> handlers;
            std::vector<std::function<void()>> deferred;
        };

        explicit EventBus(std::shared_ptr<State> state)
            : state(std::move(state))
        {
        }

        static void unsubscribe(State& state, const std::type_index& type, std::size_t id)
        {
            auto found = state.handlers.find(type);

            if (found == state.handlers.end())
            {
                return;
            }

            for (auto& handler : found->second)
            {
                if (handler.id == id)
                {
                    handler.active = false;
                    break;
                }
            }

            compactHandlers(state, type);
        }

        static void compactHandlers(State& state, const std::type_index& type)
        {
            if (state.emitting != 0)
            {
                return;
            }

            auto found = state.handlers.find(type);

            if (found == state.handlers.end())
            {
                return;
            }

            auto& handlers = found->second;
            handlers.erase(
                std::remove_if(handlers.begin(), handlers.end(), [](const Handler& handler) {
                    return !handler.active;
                }),
                handlers.end());

            if (handlers.empty())
            {
                state.handlers.erase(found);
            }
        }

        std::shared_ptr<State> state;
    };

    class EventWorld
    {
    public:
        EventBus& engine()
        {
            return engineEvents;
        }

        EventBus& scene()
        {
            return sceneEvents;
        }

        EventBus& physics()
        {
            return physicsEvents;
        }

        void flush()
        {
            engineEvents.flush();
            sceneEvents.flush();
            physicsEvents.flush();
        }

    private:
        EventBus engineEvents;
        EventBus sceneEvents;
        EventBus physicsEvents;
    };
}
