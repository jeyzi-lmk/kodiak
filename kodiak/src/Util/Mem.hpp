#pragma once

namespace Mem {
    template <typename T> T __forceinline static read_value(HANDLE handle, void* src) {
        T value = {};
        DWORD protect;
        VirtualProtectEx(handle, src, sizeof(T), PAGE_EXECUTE_READWRITE, &protect);
        NT::Read(handle, src, &value, sizeof(T), nullptr);
        VirtualProtectEx(handle, src, sizeof(T), protect, &protect);

        return value;
    }

    __forceinline static std::vector<BYTE> read_bytes(HANDLE handle, void* src, size_t size) {
        std::vector<BYTE> buffer(size);
        DWORD protect;
        VirtualProtectEx(handle, src, size, PAGE_EXECUTE_READWRITE, &protect);
        NT::Read(handle, src, buffer.data(), size, nullptr);
        VirtualProtectEx(handle, src, size, protect, &protect);
        return buffer;
    }

    template <typename T> T __forceinline static write_value(HANDLE handle, void* src, T value) {
        DWORD protect;
        VirtualProtectEx(handle, src, sizeof(T), PAGE_EXECUTE_READWRITE, &protect);
        NT::Write(handle, src, &value, sizeof(T), nullptr);
        VirtualProtectEx(handle, src, sizeof(T), protect, &protect);
        return value;
    }

    __forceinline static void write_bytes(HANDLE handle, void* dst, std::vector<BYTE> bytes) {
        DWORD protect;
        VirtualProtectEx(handle, dst, bytes.size(), PAGE_EXECUTE_READWRITE, &protect);
        NT::Write(handle, dst, bytes.data(), bytes.size(), nullptr);
        VirtualProtectEx(handle, dst, bytes.size(), protect, &protect);
    }

    __forceinline static void write_nop(HANDLE handle, void* dst, size_t size) {
        DWORD protect;
        VirtualProtectEx(handle, dst, size, PAGE_EXECUTE_READWRITE, &protect);
        std::vector<uint8_t> bytes(size, 0x90);
        NT::Write(handle, dst, bytes.data(), size, nullptr);
        VirtualProtectEx(handle, dst, size, protect, &protect);
    }

    __forceinline static void write_zero(HANDLE handle, void* dst, size_t size) {
        DWORD protect;
        VirtualProtectEx(handle, dst, size, PAGE_EXECUTE_READWRITE, &protect);
        std::vector<uint8_t> bytes(size, 0x00);
        NT::Write(handle, dst, bytes.data(), size, nullptr);
        VirtualProtectEx(handle, dst, size, protect, &protect);
    }

    __forceinline static void* alloc(HANDLE handle, void *src, size_t size) {
        return VirtualAllocEx(handle, src, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }

    __forceinline static BOOL unalloc(HANDLE handle, void *src, size_t size) {
        return VirtualFreeEx(handle, src, size, MEM_FREE);
    }

    __forceinline static void* create_field(HANDLE handle, uintptr_t modAddr, size_t fieldSize) {
        uintptr_t currentAddress = modAddr - 0x30000;
        while (currentAddress < modAddr) {
            auto buffer = read_bytes(handle, reinterpret_cast<void*>(currentAddress), fieldSize);
            if(!std::all_of(buffer.begin(), buffer.end(), [](const std::uint8_t b){ return b == 0; })) {
                currentAddress += fieldSize;
                continue;
            }

            write_nop(handle, reinterpret_cast<void*>(currentAddress), fieldSize);
            return reinterpret_cast<void*>(currentAddress);
        }
        return nullptr;
    }

    __forceinline static void* create_code_cave(HANDLE handle, uintptr_t modAddr, size_t caveSize) {
        uintptr_t currentAddress = modAddr - 0x30000;
        while (currentAddress < modAddr) {
            auto buffer = read_bytes(handle, reinterpret_cast<void*>(currentAddress), caveSize);
            if(!std::all_of(buffer.begin(), buffer.end(), [](const std::uint8_t b){ return b == 0; })) {
                currentAddress += caveSize;
                continue;
            }

            write_nop(handle, reinterpret_cast<void*>(currentAddress), caveSize);
            return reinterpret_cast<void*>(currentAddress);
        }
        return nullptr;
    }

    __forceinline static void* resolve_signature(HANDLE handle, const uintptr_t& modAddr, const size_t& size, const std::string& signature) {
        Pattern::Compiler compiler = Pattern::compile(signature);

        constexpr SIZE_T chunkSize = 1024 * 1024;
        SIZE_T bytesRead;
        std::unique_ptr<BYTE[]> pageData(new BYTE[chunkSize]);

        uintptr_t currentAddress = modAddr;
        SIZE_T regionSize = size;
        while (currentAddress < modAddr + regionSize) {
            SIZE_T toRead = std::min(chunkSize, regionSize - (currentAddress - modAddr));

            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQueryEx(handle, reinterpret_cast<void*>(currentAddress), &mbi, sizeof(mbi)) == 0) {
                currentAddress += toRead;
                continue;
            }

            if (mbi.State != MEM_COMMIT || (mbi.Protect & PAGE_GUARD) || (mbi.Protect == PAGE_NOACCESS)) {
                currentAddress += mbi.RegionSize;
                continue;
            }

            if (!ReadProcessMemory(handle, reinterpret_cast<void*>(currentAddress), pageData.get(), toRead, &bytesRead) || bytesRead < compiler.bytes.size()) {
                currentAddress += toRead;
                continue;
            }

            const BYTE* patBytes = compiler.bytes.data();
            const char* patMask = compiler.mask.c_str();
            size_t bytesToCheck = bytesRead - compiler.bytes.size();

            for (size_t i = 0; i <= bytesToCheck; ++i) {
                if (Pattern::compare(pageData.get() + i, patBytes, patMask)) {
                    return reinterpret_cast<void*>(currentAddress + i);
                }
            }

            currentAddress += toRead;
        }

        return nullptr;
    }
}
