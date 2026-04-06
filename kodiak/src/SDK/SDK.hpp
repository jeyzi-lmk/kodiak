#pragma once
#include "src/Game/Game.hpp"

namespace SDK {
    inline static void* localPlayerField = nullptr;
    inline static void* hitResultActorField = nullptr;
    inline static void* cameraField = nullptr;

    __forceinline static Actor* getLocalPlayer() {
        return reinterpret_cast<Actor*>(Mem::read_value<uintptr_t>(Game::handle, localPlayerField));
    }

    __forceinline static Actor* getHitResultActor() {
        return reinterpret_cast<Actor*>(Mem::read_value<uintptr_t>(Game::handle, hitResultActorField));
    }

    __forceinline static std::optional<Camera> getCamera() {
        auto pointer = Mem::read_value<uintptr_t>(Game::handle, cameraField);
        if(!pointer) return std::nullopt;

        return Mem::read_value<Camera>(Game::handle, reinterpret_cast<void*>(pointer));
    }

    __forceinline static void setYaw(float yaw) {
        auto pointer = Mem::read_value<uintptr_t>(Game::handle, cameraField);
        if(!pointer) return;

        Mem::write_value<float>(Game::handle, reinterpret_cast<void*>(pointer + 0x0), yaw);
    }

    __forceinline static void setPitch(float pitch) {
        auto pointer = Mem::read_value<uintptr_t>(Game::handle, cameraField);
        if(!pointer) return;

        Mem::write_value<float>(Game::handle, reinterpret_cast<void*>(pointer + 0x4), pitch);
    }
}
