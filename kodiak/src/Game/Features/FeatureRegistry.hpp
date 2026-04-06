#pragma once

namespace FeatureRegistry {
    using FeatureMap = std::map<std::string, std::unique_ptr<Feature>>;

    inline static FeatureMap registry = {};

    __forceinline static void registerFeature(const std::string& name, std::unique_ptr<Feature> hook) {
        registry[name] = std::move(hook);
    }

    __forceinline static Feature* getFeature(const std::string& name) {
        if (const auto it = registry.find(name); it != registry.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    template<typename T>
    __forceinline static T* getFeature() {
        for (const auto& [name, feature] : registry) {
            if (T* casted = dynamic_cast<T*>(feature.get())) {
                return casted;
            }
        }
        return nullptr;
    }

    __forceinline static FeatureMap& getAllFeatures() {
        return registry;
    }

    __forceinline static void clear() {
        registry.clear();
    }
};
