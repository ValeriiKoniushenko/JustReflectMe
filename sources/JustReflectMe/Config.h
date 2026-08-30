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
#include <set>
#include <vector>

namespace Yaml
{
    class Node;
}

namespace JRM
{
    class BaseConfigParam;

    /**
     * @brief Base type for a collection of named configuration parameters.
     */
    class BaseConfig
    {
    public:
        BaseConfig(const BaseConfig&) = default;
        BaseConfig& operator=(const BaseConfig&) = default;
        BaseConfig(BaseConfig&&) noexcept = default;
        BaseConfig& operator=(BaseConfig&&) noexcept = default;
        virtual ~BaseConfig() = default;
        /**
         * @brief Returns the parameters owned by this configuration.
         * @return The configuration parameter list.
         */
        [[nodiscard]] const std::vector<std::shared_ptr<BaseConfigParam>>& getParams()
            const noexcept;

    protected:
        BaseConfig() = default;

        std::vector<std::shared_ptr<BaseConfigParam>> _params;
    };

    /**
     * @brief Describes a configuration parameter independently of its value type.
     */
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

    /**
     * @brief Stores a typed configuration value together with its name and description.
     * @tparam ValueT The type of the configuration value.
     */
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

    /**
     * @brief Configuration options used by the JustReflectMe file scanner and generator.
     *
     * The parameter members expose both the option metadata and the mutable value through their
     * `value` member. Defaults are also used when `Config::jrmFolder/Config::jrmConfig` is
     * created automatically.
     */
    struct Config : public BaseConfig
    {
        Config();
        Config(const Config&) = default;
        Config& operator=(const Config&) = default;
        Config(Config&&) noexcept = default;
        Config& operator=(Config&&) noexcept = default;
        ~Config() override = default;

        /**
         * @brief Directory below the project root containing JustReflectMe state.
         */
        constexpr static std::string_view jrmFolder = ".jrm";

        /**
         * @brief Configuration filename inside `Config::jrmFolder`.
         */
        constexpr static std::string_view jrmConfig = "config.yaml";

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

        PARAM(std::vector<std::string>, rFriendAliases,
              "Put the alias(es) for R_FRIEND to be able to extend the default behavior. It will "
              "be assumed that you included this alias deeper in your system.",
              { "R_FRIEND" });
    };
#undef PARAM

    class ConfigManager final
    {
    public:
        /**
         * @brief Loads a project's configuration, creating a default file when needed.
         * @param projectDir Project root containing the optional
         * `Config::jrmFolder/Config::jrmConfig` file.
         * @param hasError Set to `true` when the configuration file cannot be parsed.
         * @return The loaded configuration, including defaults for omitted options.
         */
        [[nodiscard]] Config initializeProjectAndLoadConfig(const std::filesystem::path& projectDir,
                                                            bool& hasError);

        /**
         * @brief Builds the default YAML configuration text.
         * @return A YAML document containing the default configuration and parameter comments.
         */
        [[nodiscard]] static std::string spawnFallbackConfigAsString();

    private:
        void spawnFallbackFileConfig();

        static void validateTopLevelFields(const Yaml::Node& config);

    private:
        std::filesystem::path _projectDir;
    };
} // namespace JRM
