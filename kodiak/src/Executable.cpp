#pragma once
#include "Game/Runtime.hpp"

void commandLoop() {
    std::string line;
    while (std::getline(std::cin, line) && Runtime::running.load(std::memory_order_acquire)) {
        std::cout << "> " << std::flush;

        if (line.empty()) continue;
        if (!line.empty() && line[0] == '/') line.erase(line.begin());

        std::istringstream iss(line);
        std::vector<std::string> toks;
        for (std::string t; iss >> t; ) toks.push_back(t);
        if (toks.empty()) continue;

        Commands::Command cmd;
        cmd.raw  = line;
        cmd.name = toks[0];
        std::transform(cmd.name.begin(), cmd.name.end(), cmd.name.begin(), ::tolower);

        for (size_t i = 1; i < toks.size(); ++i) {
            const std::string& tok = toks[i];
            if (tok == "true" || tok == "false") { cmd.args.emplace_back(tok == "true"); continue; }
            char* endI=nullptr; long long iv = std::strtoll(tok.c_str(), &endI, 10);
            if (*endI == '\0') { cmd.args.emplace_back(int64_t(iv)); continue; }
            char* endD=nullptr; double dv = std::strtod(tok.c_str(), &endD);
            if (*endD == '\0') { cmd.args.emplace_back(dv); continue; }
            cmd.args.emplace_back(tok);
        }

        Commands::Dispatcher::post(std::move(cmd));
    }
}

void stopProcess() {
    if (auto* feature = FeatureRegistry::getFeature<AimAssist>(); feature != nullptr) {
        Config::captureAimAssistSettings(*feature);
        Config::save();
    }

    Commands::Dispatcher::shutdown();

    Runtime::running.store(false, std::memory_order_release);
    if (Runtime::stopEvent) {
        SetEvent(Runtime::stopEvent);
    }

    if (Runtime::handle) {
        WaitForSingleObject(Runtime::handle, INFINITE);
        CloseHandle(Runtime::handle);
        Runtime::handle = nullptr;
    }

    if (Runtime::stopEvent) {
        CloseHandle(Runtime::stopEvent);
        Runtime::stopEvent = nullptr;
    }

    Diagnostics::info("Application shutting down.");
    Diagnostics::shutdown();
    Console::shutdown();
}

BOOL WINAPI CtrlHandler(DWORD evt) {
    switch (evt) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT: {
            stopProcess();
            return TRUE;
        }
        default:
            return FALSE;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_QUERYENDSESSION:
        case WM_ENDSESSION:
        case WM_CLOSE:
        case WM_DESTROY: {
            stopProcess();
            return 0;
        }
        default: break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

auto wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) -> int {
    if(!Console::init(L"External AimAssist", true))
        return -1;

    Diagnostics::init();
    Diagnostics::info("Application started.");
    Config::load();

    SetConsoleCtrlHandler(CtrlHandler, TRUE);
    std::atexit(stopProcess);

    Console::print("Waiting for game process...");
    while (!Game::isPresent())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Console::clear();
    Console::print("Creating stop event...");
    Runtime::stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!Runtime::stopEvent) {
        Console::print("Failed to create stop event.");
        Console::pause();
        Console::shutdown();
        return 1;
    }

    Commands::Dispatcher::init();
    Runtime::running.store(true, std::memory_order_release);

    Console::print("Starting runtime thread...");
    Runtime::handle = CreateThread(nullptr, 0, &Runtime::RuntimeThreadProc, nullptr, 0, nullptr);
    if (!Runtime::handle) {
        Console::print("Failed to create runtime thread.");
        Console::pause();
        Console::shutdown();
        return 1;
    }

    commandLoop();

    Runtime::running.store(false, std::memory_order_release);
    SetEvent(Runtime::stopEvent);
    WaitForSingleObject(Runtime::handle, INFINITE);

    stopProcess();
    return 0;
}
