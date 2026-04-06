#pragma once

namespace Asm {
    __forceinline std::vector<BYTE> jmp(uintptr_t from, uintptr_t to) {
        auto offset = static_cast<int32_t>(to - (from + 5));
        std::vector<BYTE> jmpBytes = {
            0xE9,
            static_cast<BYTE>(offset & 0xFF),
            static_cast<BYTE>((offset >> 8) & 0xFF),
            static_cast<BYTE>((offset >> 16) & 0xFF),
            static_cast<BYTE>((offset >> 24) & 0xFF)
        };

        return jmpBytes;
    }

    __forceinline static std::vector<BYTE> mov_rax(uintptr_t address) {
        std::vector<BYTE> dynamicBytes = {0x48, 0xA3};

        for (int i = 0; i < sizeof(address); ++i) {
            dynamicBytes.push_back(static_cast<BYTE>((address >> (8 * i)) & 0xFF));
        }

        return dynamicBytes;
    }

    __forceinline static std::vector<BYTE> lea(uint32_t offset = 0) {
        std::vector<BYTE> bytes;

        bytes.push_back(0x49);
        bytes.push_back(0x8D);
        bytes.push_back(0x85);

        bytes.push_back(static_cast<BYTE>(offset & 0xFF));
        bytes.push_back(static_cast<BYTE>((offset >> 8) & 0xFF));
        bytes.push_back(static_cast<BYTE>((offset >> 16) & 0xFF));
        bytes.push_back(static_cast<BYTE>((offset >> 24) & 0xFF));
        return bytes;
    }
}