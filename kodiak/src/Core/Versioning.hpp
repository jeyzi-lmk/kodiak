#pragma once

namespace Versioning {
    struct ProcessVersionInfo {
        std::string executablePath;
        std::string fileVersion = "unknown";
        bool detected = false;
    };

    __forceinline static std::string queryFileVersion(const std::string& path) {
        DWORD handle = 0;
        const DWORD versionSize = GetFileVersionInfoSizeA(path.c_str(), &handle);
        if (versionSize == 0) {
            return "unknown";
        }

        std::vector<char> versionBuffer(versionSize);
        if (!GetFileVersionInfoA(path.c_str(), handle, versionSize, versionBuffer.data())) {
            return "unknown";
        }

        VS_FIXEDFILEINFO* versionInfo = nullptr;
        UINT len = 0;
        if (!VerQueryValueA(versionBuffer.data(), "\\", reinterpret_cast<LPVOID*>(&versionInfo), &len) || versionInfo == nullptr) {
            return "unknown";
        }

        std::ostringstream oss;
        oss
            << HIWORD(versionInfo->dwFileVersionMS) << '.'
            << LOWORD(versionInfo->dwFileVersionMS) << '.'
            << HIWORD(versionInfo->dwFileVersionLS) << '.'
            << LOWORD(versionInfo->dwFileVersionLS);
        return oss.str();
    }

    __forceinline static ProcessVersionInfo queryAttachedProcess(HANDLE processHandle, HMODULE module) {
        ProcessVersionInfo result{};
        if (processHandle == nullptr || module == nullptr) {
            return result;
        }

        char pathBuffer[MAX_PATH] = {};
        if (GetModuleFileNameExA(processHandle, module, pathBuffer, static_cast<DWORD>(std::size(pathBuffer))) == 0) {
            return result;
        }

        result.executablePath = pathBuffer;
        result.fileVersion = queryFileVersion(result.executablePath);
        result.detected = true;
        return result;
    }
}
