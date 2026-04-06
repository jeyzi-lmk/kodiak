#pragma once

namespace Diagnostics {
    enum class Level {
        Info,
        Warning,
        Error
    };

    inline std::mutex logMutex;
    inline std::ofstream logFile;
    inline std::filesystem::path logFilePath;

    __forceinline static std::filesystem::path ensureAppDataDirectory() {
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
        return appDir;
    }

    __forceinline static std::string levelName(Level level) {
        switch (level) {
            case Level::Info: return "INFO";
            case Level::Warning: return "WARN";
            case Level::Error: return "ERROR";
        }

        return "INFO";
    }

    __forceinline static std::string timestampNow() {
        const auto now = std::chrono::system_clock::now();
        const std::time_t tt = std::chrono::system_clock::to_time_t(now);

        std::tm localTm{};
        localtime_s(&localTm, &tt);

        std::ostringstream oss;
        oss << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    __forceinline static void init() {
        std::scoped_lock lock(logMutex);
        if (logFile.is_open()) {
            return;
        }

        const auto appDir = ensureAppDataDirectory();
        logFilePath = appDir / "diagnostics.log";
        logFile.open(logFilePath, std::ios::app);
    }

    __forceinline static void shutdown() {
        std::scoped_lock lock(logMutex);
        if (logFile.is_open()) {
            logFile.flush();
            logFile.close();
        }
    }

    __forceinline static void log(Level level, const std::string& message) {
        std::scoped_lock lock(logMutex);
        if (!logFile.is_open()) {
            return;
        }

        logFile << '[' << timestampNow() << "] [" << levelName(level) << "] " << message << '\n';
        logFile.flush();
    }

    __forceinline static void info(const std::string& message) {
        log(Level::Info, message);
    }

    __forceinline static void warn(const std::string& message) {
        log(Level::Warning, message);
    }

    __forceinline static void error(const std::string& message) {
        log(Level::Error, message);
    }

    __forceinline static const std::filesystem::path& path() {
        return logFilePath;
    }
}
