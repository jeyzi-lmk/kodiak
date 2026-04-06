#pragma once

class Game {
private:
    inline static std::string processName = "Minecraft";
    inline static std::string processExecutable = processName + ".Windows.exe";
public:
    inline static DWORD pid = 0;
    inline static uintptr_t moduleAddr = 0;
    inline static SIZE_T moduleSize = 0;

    inline static HANDLE handle = nullptr;
    inline static HMODULE module = nullptr;
    inline static HWND window = nullptr;

    inline static std::atomic_bool gMouseDown{false};

    __forceinline static bool isPresent() {
        return (window = FindWindow(nullptr, processName.c_str())) != nullptr;
    }

    __forceinline static bool isAttached() {
        return pid != 0 && handle != nullptr && module != nullptr && moduleAddr != 0 && moduleSize != 0 && window != nullptr;
    }

    __forceinline static bool isInGameScreen() {
        CURSORINFO cursorInfo;
        cursorInfo.cbSize = sizeof(CURSORINFO);

        return isAttached() && GetForegroundWindow() == window && GetCursorInfo(&cursorInfo) && !(cursorInfo.flags & CURSOR_SHOWING);
    }

    __forceinline static void unAttach() {
        if (handle) {
            CloseHandle(handle);
            handle = nullptr;
        }

        pid = 0;
        moduleSize = 0;
        moduleAddr = 0;

        module = nullptr;
        window = nullptr;
    }

    __forceinline static void attach() {
        auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 process;
        ZeroMemory(&process, sizeof(process));
        process.dwSize = sizeof(process);

        if (!Process32First(snapshot, &process)) {
            CloseHandle(snapshot);
            return;
        }

        do
        {
            if(std::string(process.szExeFile) != processExecutable) continue;

            pid = process.th32ProcessID;
            handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_ALL_ACCESS, FALSE, process.th32ProcessID);

            DWORD cbNeeded;
            HMODULE hMods[1024];
            if (!EnumProcessModules(handle, hMods, sizeof(hMods), &cbNeeded)) {
                CloseHandle(snapshot);
                CloseHandle(handle);

                pid = 0;
                handle = nullptr;
                module = nullptr;
                window = nullptr;
                return;
            }

            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                TCHAR szModName[MAX_PATH];
                if (!GetModuleFileNameEx(handle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
                    continue;

                TCHAR szModContain[22];
                strcpy(szModContain, processExecutable.c_str());
                if (std::basic_string(szModName).find(std::basic_string(szModContain)) != std::string::npos) {
                    module = hMods[i];

                    MODULEINFO moduleInfo;
                    if (GetModuleInformation(handle, module, &moduleInfo, sizeof(moduleInfo))) {
                        moduleAddr = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
                        moduleSize = moduleInfo.SizeOfImage;
                    }
                    break;
                }
            }
            break;
        } while (Process32Next(snapshot, &process));

        CloseHandle(snapshot);
    }

    __forceinline static void* findPattern(const std::string& sig) {
        if(!isPresent() || !isAttached()) return nullptr;
        return Mem::resolve_signature(handle, moduleAddr, moduleSize, sig);
    }

    __forceinline static Detour* createDetour(void* from, void* to, size_t size) {
        return new Detour(handle, moduleAddr, moduleSize, reinterpret_cast<uintptr_t>(from), reinterpret_cast<uintptr_t>(to), size);
    }
};