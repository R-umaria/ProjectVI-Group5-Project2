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

        inline bool LooksLikeProjectRoot(const std::filesystem::path& directory)
        {
            namespace fs = std::filesystem;
            if (directory.empty() || !fs::exists(directory) || !fs::is_directory(directory))
            {
                return false;
            }

            return fs::exists(directory / "FleetTelemetrySystem.sln")
                || (fs::exists(directory / "config/client.config.json")
                    && fs::exists(directory / "config/server.config.json"));
        }

        inline std::filesystem::path FindProjectRootDirectory()
        {
            namespace fs = std::filesystem;

            const std::vector<fs::path> seeds =
            {
                fs::current_path(),
                GetExecutableDirectory()
            };

            for (auto seed : seeds)
            {
                if (seed.empty())
                {
                    continue;
                }

                if (!fs::is_directory(seed))
                {
                    seed = seed.parent_path();
                }

                while (!seed.empty())
                {
                    if (LooksLikeProjectRoot(seed))
                    {
                        return seed;
                    }

                    const fs::path parent = seed.parent_path();
                    if (parent == seed)
                    {
                        break;
                    }
                    seed = parent;
                }
            }

            return GetExecutableDirectory();
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

            const fs::path projectRoot = FindProjectRootDirectory();
            const fs::path currentDir = fs::current_path();
            const fs::path exeDir = GetExecutableDirectory();

            const std::vector<fs::path> bases =
            {
                projectRoot,
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

            const fs::path projectRoot = FindProjectRootDirectory();
            if (!projectRoot.empty())
            {
                return (projectRoot / inputPath).lexically_normal();
            }

            return (GetExecutableDirectory() / inputPath).lexically_normal();
        }
    }
}
