

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

#pragma once

#include <filesystem>
#include <unordered_map>

namespace JRM
{

    class Cache final
    {
    public:
        constexpr static const char* jrmCacheFileName = "cache.data";

    public:
        Cache(const std::filesystem::path& projectDir, bool ignoreCacheRequests);
        Cache(const Cache&) = delete;
        Cache& operator=(const Cache&) = delete;
        Cache(Cache&&) noexcept = delete;
        Cache& operator=(Cache&&) noexcept = delete;
        ~Cache();

        [[nodiscard]] bool isNeedUpdate(const std::filesystem::path& path);
        [[nodiscard]] bool isNeedUpdate(const std::filesystem::path& path,
                                        const std::filesystem::file_time_type& time);

        void updateFile(const std::filesystem::path& path);

        void saveCache();
        void ignoreAnyCacheRequestAndSave(bool v) noexcept;

    private:
        void initializeProjectAndLoadData(const std::filesystem::path& projectDir);
        void readCache();
        void writeCache();

    private:
        std::unordered_map<std::string, std::filesystem::file_time_type> _files;
        std::filesystem::path _projectDir;
        std::filesystem::path _targetFile;

        bool _ignoreCacheRequests = false;
    };

} // namespace JRM
