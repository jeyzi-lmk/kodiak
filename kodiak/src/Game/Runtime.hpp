#pragma once
#include <src/Util/Command.hpp>

namespace Runtime {
    inline std::atomic running{true};
    inline bool requestedEjecting = false;

    inline HANDLE handle = nullptr;
    inline HANDLE stopEvent = nullptr;

    __forceinline static void unAttach() {
        for (auto& hook : HookRegistry::getAllHooks()) {
            hook.second->disable();
        }

        if (Game::handle != nullptr) {
            if (SDK::localPlayerField != nullptr) {
                Mem::write_zero(Game::handle, SDK::localPlayerField, sizeof(uintptr_t));
            }

            if (SDK::hitResultActorField != nullptr) {
                Mem::write_zero(Game::handle, SDK::hitResultActorField, sizeof(uintptr_t));
            }

            if (SDK::cameraField != nullptr) {
                Mem::write_zero(Game::handle, SDK::cameraField, sizeof(uintptr_t));
            }
        }

        SDK::localPlayerField = nullptr;
        SDK::hitResultActorField = nullptr;
        SDK::cameraField = nullptr;

        HookRegistry::clear();
        FeatureRegistry::clear();
        Commands::Dispatcher::clearHandlers();
        requestedEjecting = false;

        Game::unAttach();
        Diagnostics::info("Process detached.");
        Console::clear();
        Console::print("Process detached.");
    }

    __forceinline static void attach() {
        Game::attach();
        if(!Game::isAttached())
            return;

        requestedEjecting = false;

        Console::clear();
        Console::print("Process attached successfully.");
        Console::print("ProcessID: " + std::to_string(Game::pid));
        Console::print_addr("Module Base Address: ", Game::moduleAddr);
        Diagnostics::info("Process attached successfully. PID=" + std::to_string(Game::pid));

        const auto versionInfo = Versioning::queryAttachedProcess(Game::handle, Game::module);
        if (versionInfo.detected) {
            {
                std::scoped_lock lock(Config::configMutex);
                Config::current.lastDetectedGameVersion = versionInfo.fileVersion;
            }

            Console::print("Game Version: " + versionInfo.fileVersion);
            Diagnostics::info("Detected process version " + versionInfo.fileVersion + " from " + versionInfo.executablePath);
        } else {
            Diagnostics::warn("Unable to determine attached process version.");
        }

        SDK::localPlayerField = Mem::create_field(Game::handle, reinterpret_cast<uintptr_t>(Game::module), sizeof(uintptr_t));
        SDK::hitResultActorField = Mem::create_field(Game::handle, reinterpret_cast<uintptr_t>(Game::module), sizeof(uintptr_t));
        SDK::cameraField = Mem::create_field(Game::handle, reinterpret_cast<uintptr_t>(Game::module), sizeof(uintptr_t));

#ifdef DEBUG
        Console::print_addr("Allocated LocalPlayer field at ", reinterpret_cast<uintptr_t>(SDK::localPlayerField));
        Console::print_addr("Allocated HitResultActor field at ", reinterpret_cast<uintptr_t>(SDK::hitResultActorField));
        Console::print_addr("Allocated Camera field at ", reinterpret_cast<uintptr_t>(SDK::cameraField));
#endif

        HookRegistry::registerHook("mouseProc", std::make_unique<MouseProcHook>(Game::gMouseDown));
        HookRegistry::registerHook("localplayer", std::make_unique<LocalPlayerHook>(SDK::localPlayerField));
        HookRegistry::registerHook("hitResultActor", std::make_unique<HitResultActorHook>(SDK::hitResultActorField));
        HookRegistry::registerHook("camera", std::make_unique<CameraHook>(SDK::cameraField));
        for (auto& hook : HookRegistry::getAllHooks()) {
            hook.second->enable();
        }

        FeatureRegistry::registerFeature("aimassist", std::make_unique<AimAssist>());
        if (auto* feature = FeatureRegistry::getFeature<AimAssist>(); feature != nullptr) {
            Config::applyAimAssistSettings(*feature);
        }

        Commands::Dispatcher::registerHandler("help", [](const Commands::Command& c){
            auto names = Commands::Dispatcher::listCommands();
            if (names.empty()) {
                Console::print("No commands available.");
                return;
            }
            std::ranges::sort(names);

            Console::print("Available commands (" + std::to_string(names.size()) + "):");
            for (const auto& name : names) {
                Console::print(" - " + name);
            }
        });
        Commands::Dispatcher::registerHandler("eject", [](const Commands::Command& c){
            Console::print("Ejected from process.");
            Diagnostics::warn("Eject requested by user command.");
            requestedEjecting = true;
        });
        Commands::Dispatcher::registerHandler("status", [](const Commands::Command& c){
            Console::print("Process attached: " + std::string(Game::isAttached() ? "yes" : "no"));
            Console::print("Detected version: " + Config::current.lastDetectedGameVersion);
            Console::print("Config path: " + Config::configPath().string());
            Console::print("Diagnostics log: " + Diagnostics::path().string());
        });
        Commands::Dispatcher::registerHandler("saveconfig", [](const Commands::Command& c){
            if (auto* feature = FeatureRegistry::getFeature<AimAssist>(); feature != nullptr) {
                Config::captureAimAssistSettings(*feature);
            }

            if (Config::save()) {
                Console::print("Configuration saved.");
                Diagnostics::info("Configuration saved to " + Config::configPath().string());
                return;
            }

            Console::print("Failed to save configuration.");
            Diagnostics::error("Failed to save configuration.");
        });
        Commands::Dispatcher::registerHandler("loadconfig", [](const Commands::Command& c){
            if (!Config::load()) {
                Console::print("Failed to load configuration.");
                Diagnostics::error("Failed to load configuration.");
                return;
            }

            if (auto* feature = FeatureRegistry::getFeature<AimAssist>(); feature != nullptr) {
                Config::applyAimAssistSettings(*feature);
            }

            Console::print("Configuration loaded.");
            Diagnostics::info("Configuration loaded from " + Config::configPath().string());
        });
        Commands::Dispatcher::registerHandler("aimassist", [](const Commands::Command& c){
            if (c.args.empty()) {
                Console::print("Invalid subcommand. Usage: /aimassist <toggle|fov|distance|vertically|vspeed|hspeed|requireclick>");
                return;
            }

            const std::string* sub = Commands::arg<std::string>(c, 0);
            if (!sub) {
                Console::print("Invalid subcommand. Usage: /aimassist <toggle|fov|distance|vertically|vspeed|hspeed>");
                return;
            }

            std::string s = *sub;
            Commands::toLowerInPlace(s);

            auto* feature = FeatureRegistry::getFeature<AimAssist>();
            if (feature == nullptr) {
                Console::print("AimAssist feature not found.");
                return;
            }

            if (s == "toggle") {
                feature->setEnabled(!feature->isEnabled());
                Config::captureAimAssistSettings(*feature);
                Console::print("aimassist -> " + std::string(feature->isEnabled() ? "enabled" : "disabled"));
                return;
            }

            if (s == "fov") {
                auto v = Commands::number(c, 1);
                if (!v) {
                    Console::print("Usage: /aimassist fov <value>");
                    return;
                }

                auto angle = static_cast<float>(*v);
                if(angle < 1.0f) {
                    Console::print("fov must be at least 1.0");
                    return;
                }

                if(angle >= 360.0f) {
                    Console::print("fov must be less than 360.0");
                    return;
                }

                feature->maxAngle = static_cast<float>(*v);
                Config::captureAimAssistSettings(*feature);
                Console::print("fov -> " + std::to_string(*v));
                return;
            }

            if (s == "distance") {
                auto v = Commands::number(c, 1);
                if (!v) {
                    Console::print("Usage: /aimassist distance <value>");
                    return;
                }

                feature->distance = static_cast<float>(*v);
                Config::captureAimAssistSettings(*feature);
                Console::print("distance -> " + std::to_string(*v));
                return;
            }

            if(s == "vertically") {
                auto v = Commands::arg<bool>(c, 1);
                if (!v) {
                    Console::print("Usage: /aimassist vertically <true|false>");
                    return;
                }

                feature->vertically = *v;
                Config::captureAimAssistSettings(*feature);
                Console::print("vertically -> " + std::string(*v ? "enabled" : "disabled"));
                return;
            }

            if(s == "vspeed") {
                auto v = Commands::number(c, 1);
                if (!v) {
                    Console::print("Usage: /aimassist vspeed <value>");
                    return;
                }

                auto speed = static_cast<float>(*v);
                if(speed < 0.1f) {
                    Console::print("vspeed must be at least 0.1");
                    return;
                }

                if(speed > 10.0f) {
                    Console::print("vspeed must be at most 10.0");
                    return;
                }

                feature->vSpeed = speed;
                Config::captureAimAssistSettings(*feature);
                Console::print("vspeed -> " + std::to_string(*v));
                return;
            }

            if(s == "hspeed") {
                auto v = Commands::number(c, 1);
                if (!v) {
                    Console::print("Usage: /aimassist hspeed <value>");
                    return;
                }

                auto speed = static_cast<float>(*v);
                if(speed < 0.1f) {
                    Console::print("hspeed must be at least 0.1");
                    return;
                }

                if(speed > 10.0f) {
                    Console::print("hspeed must be at most 10.0");
                    return;
                }

                feature->hSpeed = speed;
                Config::captureAimAssistSettings(*feature);
                Console::print("hspeed -> " + std::to_string(*v));
                return;
            }

            if(s == "requireclick") {
                auto v = Commands::arg<bool>(c, 1);
                if (!v) {
                    Console::print("Usage: /aimassist requireclick <true|false>");
                    return;
                }

                feature->onlyWhileClicking = *v;
                Config::captureAimAssistSettings(*feature);
                Console::print("requireclick -> " + std::string(*v ? "enabled" : "disabled"));
                return;
            }

            Console::print("Unknown subcommand: " + *sub + ". Usage: /aimassist <toggle|fov <val>|distance <val>>");
        });

        Console::clear();
        Console::print("Injected successfully!");
        Console::print("Features:");
        for (const auto& [name, feature] : FeatureRegistry::getAllFeatures()) {
            Console::print(" - " + name + ": " + feature->getDescription());
        }

        Console::print("");
        Console::print("Type 'help' for a list of commands.");
    }

    __forceinline DWORD WINAPI RuntimeThreadProc(LPVOID lpParam) {
        while (!stopEvent) std::this_thread::sleep_for(std::chrono::milliseconds(10));

        Console::print("Creating high-resolution timer...");
        HANDLE hTimer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        if (!hTimer) hTimer = CreateWaitableTimerW(nullptr, FALSE, nullptr);
        if (!hTimer)
            return 1;

        constexpr LONG periodMs = 1;
        LARGE_INTEGER due{};
        due.QuadPart = -static_cast<LONGLONG>(periodMs) * 10000LL;
        SetWaitableTimer(hTimer, &due, periodMs, nullptr, nullptr, FALSE);

        const HANDLE waits[3] = { stopEvent, Commands::Dispatcher::eventHandle(), hTimer };

        Console::print("Entering main runtime loop...");
        int tickCount = 0;
        for (;;) {
            if(requestedEjecting) {
                running.store(false, std::memory_order_release);
                break;
            }

            if (!Game::isAttached()) {
                if (Game::isPresent()) {
                    attach();
                } else {
                    unAttach();
                }
            } else {
                if (!Game::isPresent()) {
                    unAttach();
                }
            }

            if (!running.load(std::memory_order_acquire)) {
                DWORD w = WaitForSingleObject(stopEvent, 100);
                if (w == WAIT_OBJECT_0) {
                    break;
                }

                continue;
            }

            DWORD w = WaitForMultipleObjects(3, waits, FALSE, INFINITE);
            switch (w) {
                case WAIT_OBJECT_0: {
                    break;
                }
                case WAIT_OBJECT_0 + 1: {
                    bool ok = Commands::Dispatcher::drain();
                    if (!ok) Console::print("Failed to process command queue.");
                    break;
                }
                case WAIT_OBJECT_0 + 2: {
                    if(!Game::isAttached()) break;
                    for (auto& f : FeatureRegistry::getAllFeatures()) f.second->tick(tickCount);
                    ++tickCount;
                    break;
                }
                default: {
                    DWORD err = GetLastError();
                    Console::print("WaitForMultipleObjects failed: " + std::to_string(err));
                    Diagnostics::error("WaitForMultipleObjects failed with error " + std::to_string(err));
                    break;
                }
            }
        }

        if(Game::isAttached()) {
            Console::print("Detaching from process...");
            unAttach();
        }

        Console::print("Exiting runtime loop...");
        CancelWaitableTimer(hTimer);
        CloseHandle(hTimer);
        return 0;
    }
} // namespace Runtime
