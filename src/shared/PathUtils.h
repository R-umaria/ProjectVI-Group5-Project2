#pragma once

#include <string>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace FleetTelemetry
{
    namespace PathUtils
    {
        inline std::filesystem::path GetExecutablePath()
        {
#ifdef _WIN32
            char buffer[MAX_PATH] = {};
            const DWORD length = ::GetModuleFileNameA(nullptr, buffer, MAX_PATH);
            if (length > 0)
            {
                return std::filesystem::path(std::string(buffer, length));
            }
            return std::filesystem::current_path();
#else
            char buffer[4096] = {};
            const ssize_t length = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
            if (length > 0)
            {
                buffer[length] = '\0';
                return std::filesystem::path(buffer);
            }
            return std::filesystem::current_path();
#endif
        }

        inline std::filesystem::path GetExecutableDirectory()
        {
            const auto executablePath = GetExecutablePath();
            if (std::filesystem::is_directory(executablePath))
            {
                return executablePath;
            }
            return executablePath.parent_path();
        }

        inline std::filesystem::path ResolveExistingPath(const std::string& configuredPath)
        {
            namespace fs = std::filesystem;

            if (configuredPath.empty())
            {
                return {};
            }

            const fs::path inputPath(configuredPath);
            if (inputPath.is_absolute() && fs::exists(inputPath))
            {
                return inputPath;
            }

            const fs::path currentDir = fs::current_path();
            const fs::path exeDir = GetExecutableDirectory();

            const std::vector<fs::path> bases =
            {
                currentDir,
                exeDir,
                exeDir.parent_path(),
                exeDir.parent_path().parent_path(),
                currentDir.parent_path(),
                currentDir.parent_path().parent_path()
            };

            for (const auto& base : bases)
            {
                if (base.empty())
                {
                    continue;
                }

                const fs::path candidate = base / inputPath;
                if (fs::exists(candidate))
                {
                    return candidate.lexically_normal();
                }
            }

            return inputPath;
        }

        inline std::filesystem::path ResolveWritablePath(const std::string& configuredPath)
        {
            namespace fs = std::filesystem;

            if (configuredPath.empty())
            {
                return {};
            }

            const fs::path inputPath(configuredPath);
            if (inputPath.is_absolute())
            {
                return inputPath;
            }

            const fs::path currentDir = fs::current_path();
            const fs::path exeDir = GetExecutableDirectory();

            const std::vector<fs::path> bases =
            {
                currentDir,
                exeDir.parent_path().parent_path(),
                exeDir,
                currentDir.parent_path().parent_path()
            };

            for (const auto& base : bases)
            {
                if (base.empty())
                {
                    continue;
                }

                const fs::path candidate = (base / inputPath).lexically_normal();
                const fs::path parent = candidate.parent_path();
                if (parent.empty() || fs::exists(parent))
                {
                    return candidate;
                }
            }

            return (exeDir / inputPath).lexically_normal();
        }
    }
}
