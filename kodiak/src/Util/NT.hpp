#pragma once

namespace NT {
    using NtReadVirtualMemory_t = NTSTATUS(NTAPI*)(
        HANDLE ProcessHandle,
        PVOID BaseAddress,
        PVOID Buffer,
        ULONG NumberOfBytesToRead,
        PULONG NumberOfBytesReaded
    );

    using NtWriteVirtualMemory_t = NTSTATUS(NTAPI*)(
            HANDLE ProcessHandle,
            PVOID BaseAddress,
            PVOID Buffer,
            ULONG NumberOfBytesToWrite,
            PULONG NumberOfBytesWritten
        );

    inline NtReadVirtualMemory_t Read = reinterpret_cast<NtReadVirtualMemory_t>(
        GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtReadVirtualMemory")
    );

    inline NtWriteVirtualMemory_t Write = reinterpret_cast<NtWriteVirtualMemory_t>(
        GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWriteVirtualMemory")
    );
}