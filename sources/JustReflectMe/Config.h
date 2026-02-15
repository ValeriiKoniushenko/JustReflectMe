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

#pragma once

#include <filesystem>
#include <set>
#include <vector>

namespace Yaml
{
    class Node;
}

namespace JRM
{
    struct Config
    {
        constexpr static std::string_view propName_excludedPaths = "excludedPaths";
        std::set<std::filesystem::path> excludedPaths;

        constexpr static std::string_view propName_parsableFileExtensions
            = "parsableFileExtensions";
        std::vector<std::string> parsableFileExtensions = { ".h", ".hpp" };

        constexpr static std::string_view propName_namespace = "namespace";
        std::string namespaceName = "R";

        constexpr static std::string_view propName_showEveryIteratedFilePath
            = "showEveryIteratedFilePath";
        bool showEveryIteratedFilePath = false;

        constexpr static std::string_view propName_showSkippedFiles = "showSkippedFiles";
        bool showSkippedFiles = false;
    };

    class ConfigManager final
    {
    public:
        constexpr static std::string_view jrmFolder = ".jrm";
        constexpr static std::string_view jrmConfig = "config.yaml";
        constexpr static std::string_view jrmFallbackConfig = "";

    public:
        [[nodiscard]] Config initializeProjectAndLoadConfig(
            const std::filesystem::path& projectDir);

    private:
        void spawnFallbackFileConfig();
        [[nodiscard]] static std::string spawnFallbackConfigAsString();

        static void validateTopLevelFields(const Yaml::Node& config);

    private:
        std::filesystem::path _projectDir;
    };
} // namespace JRM