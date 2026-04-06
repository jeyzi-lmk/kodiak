#pragma once

namespace Console {
    inline std::atomic_bool g_opened{false};

    __forceinline bool init(const wchar_t* title, const bool tryAttachFirst = false) {
        if (g_opened.exchange(true)) return true;

        BOOL attached = FALSE;
        if (tryAttachFirst) {
            attached = AttachConsole(ATTACH_PARENT_PROCESS);
        }
        if (!attached) {
            if (!AllocConsole()) { g_opened = false; return false; }
        }

        if (title) SetConsoleTitleW(title);
        SetConsoleCP(65001);
        SetConsoleOutputCP(65001);

        FILE* f;
        freopen_s(&f, "CONIN$",  "r",  stdin);
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut && hOut != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                mode |= ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, mode);
            }
        }

        if (HWND wnd = GetConsoleWindow()) ShowWindow(wnd, SW_SHOW);
        return true;
    }

    __forceinline void shutdown() {
        if (!g_opened.exchange(false)) return;

        fflush(stdout); fflush(stderr);
        FILE* f = nullptr;
        freopen_s(&f, "NUL", "w", stdout);
        freopen_s(&f, "NUL", "w", stderr);
        freopen_s(&f, "NUL", "r", stdin);
        FreeConsole();
    }

    __forceinline void print(std::string text, bool newline = true) {
        if (newline) {
            std::printf("%s\n", text.c_str());
        } else {
            std::printf("%s", text.c_str());
        }
        std::fflush(stdout);
    }

    __forceinline void print_addr(std::string text, uintptr_t addr) {
        std::stringstream ss;
        ss << std::fixed << std::hex << addr;
        std::cout << text << ss.str() << std::endl;
        std::cout.flush();
    }

    __forceinline void clear() {
        system("cls");
    }

    __forceinline void pause() {
        std::cin.get();
    }
}
