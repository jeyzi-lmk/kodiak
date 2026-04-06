#pragma once
#include "Mem.hpp"

class Detour {
private:
    bool initialized = false;
    HANDLE handle{};
    uintptr_t modAddr{};
    size_t modSize{};

    uintptr_t target{};
    uintptr_t dest{};
    size_t size{};

    void* cave{};
    uintptr_t placeholderAddress{};
    std::vector<BYTE> bytes = {};
    std::vector<BYTE> caveBytes = {};
    std::vector<BYTE> placeholderBytes = {};
public:
    Detour(HANDLE handle, uintptr_t modAddr, size_t modSize, uintptr_t target, uintptr_t dest, size_t size) {
        this->handle = handle;
        this->modAddr = modAddr;
        this->modSize = modSize;
        this->target = target;
        this->dest = dest;
        this->size = size;
    }

    void enable() {
        if(initialized) return;

        bytes = Mem::read_bytes(handle, reinterpret_cast<void*>(target), size);
        cave = Mem::create_code_cave(handle, modAddr, size + 10);
        if (cave == nullptr) {
            return;
        }

        caveBytes = Mem::read_bytes(handle, cave, size + 10);

        constexpr size_t placeholderSize = 5;
        constexpr size_t maxPlaceholderScan = 128;
        placeholderAddress = 0;
        for (size_t offset = 0; offset < maxPlaceholderScan; ++offset) {
            auto buffer = Mem::read_bytes(handle, reinterpret_cast<void*>(dest + offset), placeholderSize);
            if (std::all_of(buffer.begin(), buffer.end(), [](const std::uint8_t b){ return b == 0x11; })) {
                placeholderAddress = dest + offset;
                placeholderBytes = buffer;
                break;
            }
        }

        if (placeholderAddress == 0) {
            cave = nullptr;
            caveBytes.clear();
            return;
        }

        std::vector<BYTE> shellCode = {};
        shellCode.insert(shellCode.end(), bytes.begin(), bytes.end());

        std::vector<BYTE> jmpToDest = Asm::jmp(reinterpret_cast<uintptr_t>(cave) + shellCode.size(), dest);
        shellCode.insert(shellCode.end(), jmpToDest.begin(), jmpToDest.end());

        std::vector<BYTE> jmpToDetour = Asm::jmp(placeholderAddress, reinterpret_cast<uintptr_t>(cave) + shellCode.size());
        Mem::write_bytes(handle, reinterpret_cast<void*>(placeholderAddress), jmpToDetour);

        std::vector<BYTE> jmpBack = Asm::jmp(reinterpret_cast<uintptr_t>(cave) + shellCode.size(), target + size);
        shellCode.insert(shellCode.end(), jmpBack.begin(), jmpBack.end());
        Mem::write_bytes(handle, cave, shellCode);

        std::vector<BYTE> jmpToCave = Asm::jmp(target, reinterpret_cast<uintptr_t>(cave));
        if (size > 5) {
            jmpToCave.insert(jmpToCave.end(), size - 5, 0x90);
        }
        Mem::write_bytes(handle, reinterpret_cast<void*>(target), jmpToCave);
        initialized = true;
    }

    void disable() {
        if(!initialized) return;
        initialized = false;

        Mem::write_bytes(handle, reinterpret_cast<void*>(target), bytes);
        if (placeholderAddress != 0 && !placeholderBytes.empty()) {
            Mem::write_bytes(handle, reinterpret_cast<void*>(placeholderAddress), placeholderBytes);
        }

        if (cave != nullptr && !caveBytes.empty()) {
            Mem::write_bytes(handle, cave, caveBytes);
        }

        placeholderAddress = 0;
        placeholderBytes.clear();
        caveBytes.clear();
        cave = nullptr;
    }
};
