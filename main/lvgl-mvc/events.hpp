#pragma once

#include <functional>
#include <type_traits>

#include "lvgl-mvc.hpp"

#define DEFINE_EVENTS_ENUM(Enum)                                    \
    inline constexpr Enum operator|(Enum Lhs, Enum Rhs)             \
    {                                                               \
        return static_cast<Enum>(                                   \
            static_cast<std::underlying_type_t<Enum>>(Lhs) |        \
            static_cast<std::underlying_type_t<Enum>>(Rhs));        \
    }                                                               \
    inline constexpr Enum operator&(Enum Lhs, Enum Rhs)             \
    {                                                               \
        return static_cast<Enum>(                                   \
            static_cast<std::underlying_type_t<Enum>>(Lhs) &        \
            static_cast<std::underlying_type_t<Enum>>(Rhs));        \
    }                                                               \
    inline constexpr Enum operator^(Enum Lhs, Enum Rhs)             \
    {                                                               \
        return static_cast<Enum>(                                   \
            static_cast<std::underlying_type_t<Enum>>(Lhs) ^        \
            static_cast<std::underlying_type_t<Enum>>(Rhs));        \
    }                                                               \
    inline constexpr Enum operator~(Enum E)                         \
    {                                                               \
        return static_cast<Enum>(                                   \
            ~static_cast<std::underlying_type_t<Enum>>(E));         \
    }                                                               \
    inline Enum &operator|=(Enum &Lhs, Enum Rhs)                    \
    {                                                               \
        return Lhs = static_cast<Enum>(                             \
                   static_cast<std::underlying_type_t<Enum>>(Lhs) | \
                   static_cast<std::underlying_type_t<Enum>>(Lhs)); \
    }                                                               \
    inline Enum &operator&=(Enum &Lhs, Enum Rhs)                    \
    {                                                               \
        return Lhs = static_cast<Enum>(                             \
                   static_cast<std::underlying_type_t<Enum>>(Lhs) & \
                   static_cast<std::underlying_type_t<Enum>>(Lhs)); \
    }                                                               \
    inline Enum &operator^=(Enum &Lhs, Enum Rhs)                    \
    {                                                               \
        return Lhs = static_cast<Enum>(                             \
                   static_cast<std::underlying_type_t<Enum>>(Lhs) ^ \
                   static_cast<std::underlying_type_t<Enum>>(Lhs)); \
    }

typedef void *EventSubscription;

template <typename SourceType, typename EventsType, typename EventsDataType>
class Events
{
   private:
    SourceType &source;
    class EventHandlerRegistration
    {
       public:
        EventHandlerRegistration(
            EventsType interrestMask, void *userData,
            std::function<void(SourceType &source, EventsType event,
                               EventsDataType *eventData, void *userData)>
                handler)
        {
            this->interrestMask = interrestMask;
            this->userData = userData;
            this->handler = handler;
            this->nextHandler = nullptr;
        }
        EventsType interrestMask;
        void *userData;
        std::function<void(SourceType &source, EventsType event,
                           EventsDataType *eventData, void *userData)>
            handler;
        EventHandlerRegistration *nextHandler;
    };

    EventHandlerRegistration *firstHandlerRegistration = nullptr;

   public:
    Events(SourceType &source) : source(source) {}

    EventSubscription addHandler(
        EventsType interrestMask, void *userData,
        std::function<void(SourceType &source, EventsType event,
                           EventsDataType *eventData, void *userData)>
            handler)
    {
        EventHandlerRegistration **pHandlerRegistration =
            &this->firstHandlerRegistration;

        while (*pHandlerRegistration != nullptr) {
            pHandlerRegistration = &(*pHandlerRegistration)->nextHandler;
        }

        *pHandlerRegistration =
            new EventHandlerRegistration(interrestMask, userData, handler);

        return *pHandlerRegistration;
    }

    void removeHandler(EventSubscription subscription)
    {
        EventHandlerRegistration **pHandlerRegistration =
            &this->firstHandlerRegistration;

        while (*pHandlerRegistration != nullptr &&
               *pHandlerRegistration != subscription) {
            pHandlerRegistration = &(*pHandlerRegistration)->nextHandler;
        }

        if (*pHandlerRegistration == subscription) {
            *pHandlerRegistration = (*pHandlerRegistration)->nextHandler;

            delete (EventHandlerRegistration *)subscription;
        }
    }

    void send(EventsType event, EventsDataType *eventData)
    {
        if (lvgl_mvc_lock(0)) {
            EventHandlerRegistration *handlerRegistration =
                this->firstHandlerRegistration;

            while (handlerRegistration != nullptr) {
                if (static_cast<std::underlying_type_t<EventsType>>(
                        handlerRegistration->interrestMask & event) > 0) {
                    handlerRegistration->handler(source, event, eventData,
                                                 handlerRegistration->userData);

                    handlerRegistration = handlerRegistration->nextHandler;
                }
            }

            lvgl_mvc_unlock();
        }
    }
};
