

/*
 * MIT License
 *
 * Copyright (c) 2018-2026 Valerii Koniushenko
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

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace JRM
{

    Cache::Cache(const std::filesystem::path& projectDir)
    {
        initializeProjectAndLoadData(projectDir);
    }

    Cache::~Cache()
    {
        saveCache();
    }

    void Cache::initializeProjectAndLoadData(const fs::path& projectDir)
    {
        _projectDir = projectDir;
        _targetFile = _projectDir / ConfigManager::jrmFolder / jrmCacheFileName;

        if (!fs::exists(_projectDir))
        {
            std::cerr << "[JustReflectMe] Project directory does not exist: "
                      << _projectDir.generic_string() << "\n";
            return;
        }

        if (!fs::exists(_projectDir / ConfigManager::jrmFolder))
        {
            std::error_code ec;
            if (!fs::create_directory(_projectDir / ConfigManager::jrmFolder, ec))
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
        if (!_files.contains(path))
        {
            return true;
        }

        return _files[path] != fs::file_time_type(fs::last_write_time(path));
    }

    bool Cache::isNeedUpdate(const std::filesystem::path& path,
                             const std::filesystem::file_time_type& time)
    {
#ifdef NDEBUG
        if (path.is_absolute()) [[unlikely]]
        {
            std::cerr << "[JustReflectMe] Error. Absolute path is passed to isNeedUpdate. File: " << path.generic_string() << "\n";
        }
#endif
        if (!_files.contains(path))
        {
            return true;
        }

        return _files[path] != time;
    }

    void Cache::updateFile(const fs::path& path)
    {
        _files[path] = fs::file_time_type(fs::last_write_time(path));
    }

    void Cache::saveCache()
    {
        writeCache();
    }

    void Cache::readCache()
    {
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

        const char* p = buffer.c_str();
        while (p && *p != '\0')
        {
            const auto* space = strchr(p, ' ');
            if (!space) [[unlikely]]
            {
                std::cerr << "[JustReflectMe] Cache file corrupted(1)S. File: " << _targetFile
                          << "\n";
                return;
            }

            int64_t lastWriteTime = 0;
            std::from_chars(p, space, lastWriteTime);
            p = space + 1;

            const char* newLine = strchr(space, '\n');
            if (!newLine) [[unlikely]]
            {
                std::cerr << "[JustReflectMe] Cache file corrupted(2). File: " << _targetFile
                          << "\n";
                return;
            }

            std::filesystem::path path(std::string_view(p, newLine - p));

            const auto sys = std::chrono::sys_time{ std::chrono::nanoseconds{ lastWriteTime } };
            _files.emplace(path.lexically_relative(_projectDir),
                           std::chrono::clock_cast<std::filesystem::file_time_type::clock>(sys));

            p = newLine + 1;
        }

        std::cout << "[JustReflectMe] Cache has read successfully.\n";
    }

    void Cache::writeCache()
    {
        if (_projectDir.empty()) [[unlikely]]
        {
            std::cerr << "[JustReflectMe] Cache file corrupted(3). File: " << _targetFile << "\n";
            return;
        }

        std::string buffer;
        buffer.reserve(1024);

        std::array<char, 32> timeAsString;

        for (const auto& [path, time] : _files)
        {
            auto sys = std::chrono::clock_cast<std::chrono::system_clock>(time);
            const int64_t unixNs
                = std::chrono::duration_cast<std::chrono::nanoseconds>(sys.time_since_epoch())
                      .count();

            const auto res = std::to_chars(timeAsString.data(),
                                           timeAsString.data() + timeAsString.size(), unixNs);
            if (res.ec != std::errc{}) [[unlikely]]
            {
                std::cerr << "[JustReflectMe] Corruption while saving cache. Failed to convert "
                             "file time to string. File: "
                          << path.generic_string() << "\n";
                continue;
            }

            buffer.append(timeAsString.data(), res.ptr - timeAsString.data());
            buffer += " ";
            buffer += path.lexically_relative(_projectDir).generic_string();
            buffer += "\n";
        }

        std::ofstream cacheFile(_targetFile);
        if (!cacheFile.is_open())
        {
            std::cerr << "[JustReflectMe] Failed to open cache file for write. File: "
                      << _targetFile << "\n";
            return;
        }

        cacheFile.write(buffer.c_str(), buffer.size());
    }

} // namespace JRM