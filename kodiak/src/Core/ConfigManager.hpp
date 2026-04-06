#pragma once

namespace Config {
    struct AimAssistSettings {
        bool enabled = false;
        bool onlyWhileClicking = true;
        bool vertically = false;
        float maxAngle = 90.0f;
        float distance = 5.0f;
        float hSpeed = 5.0f;
        float vSpeed = 5.0f;
    };

    struct AppConfig {
        int configVersion = 1;
        std::string lastDetectedGameVersion = "unknown";
        AimAssistSettings aimAssist{};
    };

    inline std::mutex configMutex;
    inline AppConfig current{};

    __forceinline static std::filesystem::path configPath() {
        PWSTR roamingPath = nullptr;
        const HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roamingPath);

        std::filesystem::path basePath = std::filesystem::current_path();
        if (SUCCEEDED(hr) && roamingPath != nullptr) {
            basePath = std::filesystem::path(roamingPath);
        }

        if (roamingPath != nullptr) {
            CoTaskMemFree(roamingPath);
        }

        const auto appDir = basePath / "Kodiak";
        std::error_code ec;
        std::filesystem::create_directories(appDir, ec);
        return appDir / "config.ini";
    }

    __forceinline static std::string trim(std::string value) {
        const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
        value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
        return value;
    }

    __forceinline static bool parseBool(std::string value, bool fallback) {
        Commands::toLowerInPlace(value);
        if (value == "1" || value == "true" || value == "yes" || value == "on") {
            return true;
        }

        if (value == "0" || value == "false" || value == "no" || value == "off") {
            return false;
        }

        return fallback;
    }

    __forceinline static float parseFloat(const std::string& value, float fallback) {
        char* end = nullptr;
        const float parsed = std::strtof(value.c_str(), &end);
        if (end != nullptr && *end == '\0') {
            return parsed;
        }

        return fallback;
    }

    __forceinline static void applyAimAssistSettings(AimAssist& feature) {
        std::scoped_lock lock(configMutex);
        feature.setEnabled(current.aimAssist.enabled);
        feature.onlyWhileClicking = current.aimAssist.onlyWhileClicking;
        feature.vertically = current.aimAssist.vertically;
        feature.maxAngle = current.aimAssist.maxAngle;
        feature.distance = current.aimAssist.distance;
        feature.hSpeed = current.aimAssist.hSpeed;
        feature.vSpeed = current.aimAssist.vSpeed;
    }

    __forceinline static void captureAimAssistSettings(const AimAssist& feature) {
        std::scoped_lock lock(configMutex);
        current.aimAssist.enabled = feature.isEnabled();
        current.aimAssist.onlyWhileClicking = feature.onlyWhileClicking;
        current.aimAssist.vertically = feature.vertically;
        current.aimAssist.maxAngle = feature.maxAngle;
        current.aimAssist.distance = feature.distance;
        current.aimAssist.hSpeed = feature.hSpeed;
        current.aimAssist.vSpeed = feature.vSpeed;
    }

    __forceinline static bool load() {
        std::scoped_lock lock(configMutex);
        std::ifstream file(configPath());
        if (!file.is_open()) {
            return false;
        }

        AppConfig loaded{};
        std::string section;
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line.starts_with('#') || line.starts_with(';')) {
                continue;
            }

            if (line.front() == '[' && line.back() == ']') {
                section = trim(line.substr(1, line.size() - 2));
                Commands::toLowerInPlace(section);
                continue;
            }

            const auto pos = line.find('=');
            if (pos == std::string::npos) {
                continue;
            }

            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            Commands::toLowerInPlace(key);

            if (section == "app") {
                if (key == "configversion") loaded.configVersion = static_cast<int>(parseFloat(value, static_cast<float>(loaded.configVersion)));
                if (key == "lastdetectedgameversion") loaded.lastDetectedGameVersion = value;
            }

            if (section == "aimassist") {
                if (key == "enabled") loaded.aimAssist.enabled = parseBool(value, loaded.aimAssist.enabled);
                if (key == "onlywhileclicking") loaded.aimAssist.onlyWhileClicking = parseBool(value, loaded.aimAssist.onlyWhileClicking);
                if (key == "vertically") loaded.aimAssist.vertically = parseBool(value, loaded.aimAssist.vertically);
                if (key == "maxangle") loaded.aimAssist.maxAngle = parseFloat(value, loaded.aimAssist.maxAngle);
                if (key == "distance") loaded.aimAssist.distance = parseFloat(value, loaded.aimAssist.distance);
                if (key == "hspeed") loaded.aimAssist.hSpeed = parseFloat(value, loaded.aimAssist.hSpeed);
                if (key == "vspeed") loaded.aimAssist.vSpeed = parseFloat(value, loaded.aimAssist.vSpeed);
            }
        }

        current = loaded;
        return true;
    }

    __forceinline static bool save() {
        std::scoped_lock lock(configMutex);
        std::ofstream file(configPath(), std::ios::trunc);
        if (!file.is_open()) {
            return false;
        }

        file << "[app]\n";
        file << "configVersion=" << current.configVersion << '\n';
        file << "lastDetectedGameVersion=" << current.lastDetectedGameVersion << "\n\n";

        file << "[aimassist]\n";
        file << "enabled=" << (current.aimAssist.enabled ? "true" : "false") << '\n';
        file << "onlyWhileClicking=" << (current.aimAssist.onlyWhileClicking ? "true" : "false") << '\n';
        file << "vertically=" << (current.aimAssist.vertically ? "true" : "false") << '\n';
        file << "maxAngle=" << current.aimAssist.maxAngle << '\n';
        file << "distance=" << current.aimAssist.distance << '\n';
        file << "hSpeed=" << current.aimAssist.hSpeed << '\n';
        file << "vSpeed=" << current.aimAssist.vSpeed << '\n';
        return true;
    }
}
