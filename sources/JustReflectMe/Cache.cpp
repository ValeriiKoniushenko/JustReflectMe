

/*
 * MIT License
 *
 * Copyright (c) 2018-2027 Valerii Koniushenko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Cache.h"

#include "Config.h"
#include "FileNavigationHelper.h"

#include <array>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>

namespace fs = std::filesystem;

using file_time = std::filesystem::file_time_type;

namespace JRM
{

    Cache::Cache(const std::filesystem::path& projectDir, bool ignoreCacheRequests)
    {
        ignoreAnyCacheRequestAndSave(ignoreCacheRequests);

        initializeProjectAndLoadData(projectDir);
    }

    Cache::~Cache()
    {
        saveCache();
    }

    void Cache::initializeProjectAndLoadData(const fs::path& projectDir)
    {
        _projectDir = projectDir;
        _targetFile = _projectDir / Config::jrmFolder / jrmCacheFileName;

        if (!fs::exists(_projectDir))
        {
            std::cerr << "[JustReflectMe] Project directory does not exist: "
                      << _projectDir.generic_string() << "\n";
            return;
        }

        if (!fs::exists(_projectDir / Config::jrmFolder))
        {
            std::error_code ec;
            if (!fs::create_directory(_projectDir / Config::jrmFolder, ec))
            {
                std::cerr << "[JustReflectMe] Failed to create jrm folder at the project's root. "
                             "Details: "
                          << ec.message() << "\n";
                return;
            }
        }

        readCache();
    }

    bool Cache::isNeedUpdate(const std::filesystem::path& path)
    {
        const auto relativePath = path.is_absolute() ? path.lexically_relative(_projectDir) : path;
        const auto cacheKey = relativePath.generic_string();
        if (!_files.contains(cacheKey))
        {
            return true;
        }

        const auto filePath = path.is_absolute() ? path : _projectDir / path;
        return _files[cacheKey] != fs::file_time_type(fs::last_write_time(filePath));
    }

    bool Cache::isNeedUpdate(const std::filesystem::path& path,
                             const std::filesystem::file_time_type& time)
    {
        const auto str = path.generic_string();
#ifdef NDEBUG
        if (path.is_absolute()) [[unlikely]]
        {
            std::cerr << "[JustReflectMe] Error. Absolute path is passed to isNeedUpdate. File: "
                      << str << "\n";
        }
#endif
        if (_ignoreCacheRequests)
        {
            return true;
        }

        if (!_files.contains(str))
        {
            return true;
        }

        const auto t = _files[str];
        return t != time;
    }

    void Cache::updateFile(const fs::path& path)
    {
        if (_ignoreCacheRequests)
        {
            return;
        }

        const auto relativePath = path.is_absolute() ? path.lexically_relative(_projectDir) : path;
        const auto filePath = path.is_absolute() ? path : _projectDir / path;
        _files[relativePath.generic_string()] = fs::file_time_type(fs::last_write_time(filePath));
    }

    void Cache::saveCache()
    {
        writeCache();
    }

    void Cache::ignoreAnyCacheRequestAndSave(bool v) noexcept
    {
        _ignoreCacheRequests = v;
    }

    void Cache::readCache()
    {
        if (_ignoreCacheRequests)
        {
            return;
        }

        if (_projectDir.empty()) [[unlikely]]
        {
            std::cerr << "[JustReflectMe] Can't read cache file. Internal error.\n";
            return;
        }

        std::ifstream cacheFile(_targetFile);
        if (!cacheFile.is_open())
        {
            return;
        }
        cacheFile.seekg(0, std::ios::end);
        std::string buffer(cacheFile.tellg(), 0);
        cacheFile.seekg(0, std::ios::beg);
        cacheFile.read(buffer.data(), buffer.size());
        cacheFile.close();

        while (!buffer.empty() && FileNavigator::IsNewLine(buffer.back()))
        {
            buffer.pop_back();
        }

        bool debugCheck_FoundNegativeTime = false;
        bool debugCheck_FoundEmptyLine = false;

        const char* p = buffer.c_str();
        while (p && *p != '\0')
        {
            if (FileNavigator::LeadToNewLine(p))
            {
                debugCheck_FoundEmptyLine = true;
                p = strchr(p, '\n') + 1;
                continue;
            }

            const auto* spaceP = strchr(p, ' ');
            if (!spaceP) [[unlikely]]
            {
                std::cerr << "[JustReflectMe] Cache file corrupted(1). Delete it manually. It will "
                             "be regenerated automatically. File: "
                          << _targetFile << "\n";
                return;
            }

            uint64_t lastWriteTime = 0;
            std::from_chars(p, spaceP, lastWriteTime);
            p = spaceP + 1;

            /*if (lastWriteTime < 0)
            {
                debugCheck_FoundNegativeTime = true;
            }*/

            const char* newLine = strchr(spaceP, '\n');
            if (!newLine)
            {
                newLine = strchr(spaceP, '\0');
                if (!newLine) [[unlikely]]
                {
                    std::cerr
                        << "[JustReflectMe] Cache file corrupted(2). Delete it manually. It will "
                           "be regenerated automatically. File: "
                        << _targetFile << "\n";
                    return;
                }
            }

            std::filesystem::path path(std::string_view(p, newLine - p));

            const auto ft = file_time{ file_time::duration{ lastWriteTime } };

            if (path.is_absolute())
            {
                path = path.lexically_relative(_projectDir);
            }

            _files.emplace(path.generic_string(), ft);

            if (*newLine == '\0')
            {
                break;
            }

            p = newLine + 1;
        }

        if (debugCheck_FoundNegativeTime)
        {
            std::cout << "[JustReflectMe] Debug checks: Found negative time value[s] in cache\n";
        }
        if (debugCheck_FoundEmptyLine)
        {
            std::cout << "[JustReflectMe] Debug checks: Found an empty line in cache\n";
        }

        // std::cout << "[JustReflectMe] Cache has read successfully.\n";
    }

    void Cache::writeCache()
    {
        if (_ignoreCacheRequests)
        {
            return;
        }

        if (_projectDir.empty()) [[unlikely]]
        {
            std::cerr << "[JustReflectMe] Cache file corrupted(3). Delete it manually. It will be "
                         "regenerated automatically. File: "
                      << _targetFile << "\n";
            return;
        }

        std::string buffer;
        buffer.reserve(1024);

        std::array<char, 32> timeAsString;

        std::map<std::filesystem::path, std::filesystem::file_time_type> files;
        for (auto&& [path, time] : _files)
        {
            if (path.empty())
            {
                std::cerr << "[JustReflectMe] Internal error: While writing the cache was found "
                             "empty path. Ignored.\n";
                continue;
            }

            files.emplace(path, time);
        }

        for (const auto& [path, time] : files)
        {
            // cross-platform: convert file_time to Unix nanoseconds via to_sys
            const uint64_t rawTicks = time.time_since_epoch().count();

            const auto res = std::to_chars(timeAsString.data(),
                                           timeAsString.data() + timeAsString.size(), rawTicks);
            if (res.ec != std::errc{}) [[unlikely]]
            {
                std::cerr << "[JustReflectMe] Corruption while saving cache. Failed to convert "
                             "file time to string. File: "
                          << path.generic_string() << "\n";
                continue;
            }

            auto finalPath = path;
            if (finalPath.is_absolute())
            {
                finalPath = path.lexically_relative(_projectDir);
            }

            buffer.append(timeAsString.data(), res.ptr - timeAsString.data());
            buffer += " ";
            buffer += finalPath.generic_string();
            buffer += "\n";
        }

        std::ofstream cacheFile(_targetFile);
        if (!cacheFile.is_open()) [[unlikely]]
        {
            std::cerr << "[JustReflectMe] Failed to open cache file for write. File: "
                      << _targetFile << "\n";
            return;
        }

        cacheFile.write(buffer.c_str(), buffer.size());
    }

} // namespace JRM
