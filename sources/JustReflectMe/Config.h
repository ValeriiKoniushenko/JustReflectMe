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
    class BaseConfigParam;

    class BaseConfig
    {
    public:
        [[nodiscard]] const std::vector<std::shared_ptr<BaseConfigParam>>& getParams()
            const noexcept;

        virtual ~BaseConfig() = default;

    protected:
        BaseConfig() = default;

        std::vector<std::shared_ptr<BaseConfigParam>> _params;
    };

    class BaseConfigParam
    {
    public:
        BaseConfigParam(const BaseConfigParam&) = default;
        BaseConfigParam(BaseConfigParam&&) = default;
        BaseConfigParam& operator=(const BaseConfigParam&) = default;
        BaseConfigParam& operator=(BaseConfigParam&&) = default;
        BaseConfigParam(std::string_view paramName, std::string_view description);
        virtual ~BaseConfigParam() = default;

    public:
        std::string_view paramName;
        std::string_view description;
    };

    template<class ValueT>
    struct Param : public BaseConfigParam
    {
        using Ptr = std::shared_ptr<Param>;

        Param(std::string_view paramName, std::string_view description, ValueT&& defaultValue)
            : BaseConfigParam(paramName, description),
              value(std::move(defaultValue))
        {
        }

        ValueT value{};
    };

    using BoolParam = Param<bool>;
    using StringParam = Param<std::string>;
    using PathSetParam = Param<std::set<std::filesystem::path>>;
    using StringVectorParam = Param<std::vector<std::string>>;

#define PARAM(type, name, description, ...)                                                        \
    Param<type>::Ptr name = std::make_shared<Param<type>>(#name, description, type{ __VA_ARGS__ })

    struct Config : public BaseConfig
    {
        constexpr static std::string_view jrmFolder = ".jrm";
        constexpr static std::string_view jrmConfig = "config.yaml";

        Config();
        ~Config() override = default;

        PARAM(std::set<std::filesystem::path>, excludedPaths,
              "Pass an array of the relative paths that should be ignored.",
              { "JustReflectMe", "build", ".vscode", ".cache", ".git", ".idea", jrmFolder });

        PARAM(std::vector<std::string>, parsableFileExtensions,
              "Array of the file extensions that will be scanned through the filesystem.",
              { ".h", ".hpp", ".hxx", ".hh", ".h++" });

        PARAM(bool, showEveryIteratedFilePath, "Show every file path while running.", false);
        PARAM(bool, showSkippedFiles, "Show skipped(no need update) files while running.", false);
        PARAM(bool, alwaysDirtyCache,
              "If true, it will always regenerate the files, regardless of whether they were "
              "modified.",
              false);
        PARAM(std::string, insertCodeAtTheTop,
              "A string will be added to the top of the generated file", "");
        PARAM(std::string, insertCodeAtTheBottom,
              "A string will be added to the bottom of the generated file", "");
        PARAM(bool, ignoreSerializationSignals,
              "Ignore signals that are used for serialization purposes. If true, only the mehtod "
              "on(Pre/Post)[De]Serialize will be called from the top called object. If false, "
              "every class will try to use only its own on(Pre/Post)[De]Serialize method.",
              true);
    };
#undef PARAM

    class ConfigManager final
    {
    public:
        [[nodiscard]] Config initializeProjectAndLoadConfig(const std::filesystem::path& projectDir,
                                                            bool& hasError);

        [[nodiscard]] static std::string spawnFallbackConfigAsString();

    private:
        void spawnFallbackFileConfig();

        static void validateTopLevelFields(const Yaml::Node& config);

    private:
        std::filesystem::path _projectDir;
    };
} // namespace JRM