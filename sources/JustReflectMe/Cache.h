// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <filesystem>
#include <unordered_map>

namespace JRM
{

    /**
     * @brief Persists the last processed timestamp of project files.
     *
     * Cache entries are stored below the project's `Config::jrmFolder` directory (currently
     * `.jrm`) at the path formed from `Config::jrmFolder` and `Cache::jrmCacheFileName` (currently
     * `cache.data`). The configuration file in the same directory is named by `Config::jrmConfig`.
     * The cache is saved when the object is destroyed and can be bypassed when regeneration is
     * required.
     */
    class Cache final
    {
    public:
        /**
         * @brief Filename used for cache data below `Config::jrmFolder`.
         */
        constexpr static const char* jrmCacheFileName = "cache.data";

    public:
        /**
         * @brief Opens or initializes the cache for a project.
         * @param projectDir Project root whose `Config::jrmFolder/Cache::jrmCacheFileName` file is
         * used.
         * @param ignoreCacheRequests If true, update checks always report that work is needed and
         * cache updates are ignored.
         */
        Cache(const std::filesystem::path& projectDir, bool ignoreCacheRequests);
        Cache(const Cache&) = delete;
        Cache& operator=(const Cache&) = delete;
        Cache(Cache&&) noexcept = delete;
        Cache& operator=(Cache&&) noexcept = delete;
        ~Cache();

        /**
         * @brief Checks whether the file's current timestamp differs from the cached timestamp.
         * @param path An absolute path or a path relative to the project root.
         * @return `true` when the file is not cached or has changed.
         */
        [[nodiscard]] bool isNeedUpdate(const std::filesystem::path& path);

        /**
         * @brief Checks whether a supplied timestamp differs from the cached timestamp.
         * @param path A path relative to the project root.
         * @param time The timestamp to compare with the cached value.
         * @return `true` when the file is not cached or the timestamps differ.
         */
        [[nodiscard]] bool isNeedUpdate(const std::filesystem::path& path,
                                        const std::filesystem::file_time_type& time);

        /**
         * @brief Records the file's current last-write timestamp.
         * @param path An absolute path or a path relative to the project root.
         */
        void updateFile(const std::filesystem::path& path);

        /**
         * @brief Writes the current cache entries to disk.
         */
        void saveCache();

        /**
         * @brief Enables or disables bypassing cache requests and updates.
         * @param v `true` to force files to be treated as needing an update.
         */
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
