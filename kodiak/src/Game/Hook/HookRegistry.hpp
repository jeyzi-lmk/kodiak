#pragma once

namespace HookRegistry {
    using HookMap = std::map<std::string, std::unique_ptr<Hook>>;

    inline static HookMap registry = {};

    __forceinline static void registerHook(const std::string& name, std::unique_ptr<Hook> hook) {
        registry[name] = std::move(hook);
    }

    __forceinline static Hook* getHook(const std::string& name) {
        if (const auto it = registry.find(name); it != registry.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    __forceinline static HookMap& getAllHooks() {
        return registry;
    }

    __forceinline static void clear() {
        registry.clear();
    }
}
