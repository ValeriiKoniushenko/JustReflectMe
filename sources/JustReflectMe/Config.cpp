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

#include "Config.h"

#include "Yaml.h"

#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace JRM
{

    const std::vector<std::shared_ptr<BaseConfigParam>>& BaseConfig::getParams() const noexcept
    {
        return _params;
    }

    BaseConfigParam::BaseConfigParam(std::string_view paramName_, std::string_view description_)
        : paramName(paramName_),
          description(description_)
    {
    }

    Config::Config()
    {
        _params.emplace_back(excludedPaths);
        _params.emplace_back(parsableFileExtensions);
        _params.emplace_back(showEveryIteratedFilePath);
        _params.emplace_back(showSkippedFiles);
        _params.emplace_back(alwaysDirtyCache);
        _params.emplace_back(insertCodeAtTheTop);
        _params.emplace_back(insertCodeAtTheBottom);
    }

    Config ConfigManager::initializeProjectAndLoadConfig(const fs::path& projectDir)
    {
        _projectDir = projectDir;

        Config config;

        try
        {
            Yaml::Node yaml;
            if (!fs::exists(_projectDir / Config::jrmFolder)
                || !fs::exists(_projectDir / Config::jrmFolder / Config::jrmConfig))
            {
                spawnFallbackFileConfig();
                Yaml::Parse(yaml, ConfigManager::spawnFallbackConfigAsString());
            }
            else
            {
                Yaml::Parse(
                    yaml,
                    (_projectDir / Config::jrmFolder / Config::jrmConfig).generic_string().c_str());
            }

            ConfigManager::validateTopLevelFields(yaml);

            if (auto&& item = yaml[config.excludedPaths->paramName.data()]; !item.IsNone())
            {
                config.excludedPaths->value.clear();
                for (auto it = item.Begin(); it != item.End(); it++)
                {
                    config.excludedPaths->value.emplace((*it).second.As<std::string>());
                }
            }

            if (auto&& item = yaml[config.parsableFileExtensions->paramName.data()]; !item.IsNone())
            {
                config.parsableFileExtensions->value.clear();
                for (auto it = item.Begin(); it != item.End(); it++)
                {
                    config.parsableFileExtensions->value.emplace_back(
                        (*it).second.As<std::string>());
                }
            }

            if (auto&& item = yaml[config.showEveryIteratedFilePath->paramName.data()];
                !item.IsNone())
            {
                config.showEveryIteratedFilePath->value = item.As<bool>();
            }

            if (auto&& item = yaml[config.showSkippedFiles->paramName.data()]; !item.IsNone())
            {
                config.showSkippedFiles->value = item.As<bool>();
            }

            if (auto&& item = yaml[config.alwaysDirtyCache->paramName.data()]; !item.IsNone())
            {
                config.alwaysDirtyCache->value = item.As<bool>();
            }

            if (auto&& item = yaml[config.insertCodeAtTheTop->paramName.data()]; !item.IsNone())
            {
                config.insertCodeAtTheTop->value = item.As<std::string>();
            }

            if (auto&& item = yaml[config.insertCodeAtTheBottom->paramName.data()]; !item.IsNone())
            {
                config.insertCodeAtTheBottom->value = item.As<std::string>();
            }
        }
        catch (Yaml::Exception& ex)
        {
            std::cerr << "[JustReflectMe] Failed to parse .yaml config file. Details: " << ex.what()
                      << std::endl;
            std::cerr << "[JustReflectMe] The JRM is configured incompletely!\n";
        }

        return config;
    }

    void ConfigManager::spawnFallbackFileConfig()
    {
        std::error_code ec;
        fs::create_directory(_projectDir / Config::jrmFolder, ec);

        if (ec)
        {
            std::cerr
                << "[JustReflectMe] Failed to create jrm folder at the project's root. Details: "
                << ec.message() << "\n";
            return;
        }

        {
            std::ofstream out(_projectDir / Config::jrmFolder / Config::jrmConfig);
            if (!out.is_open())
            {
                std::cerr
                    << "[JustReflectMe] Failed to spawn a jrm fallback config at: <project_root>/"
                    << Config::jrmFolder << "/" << Config::jrmConfig << "\n";
                return;
            }
            out << ConfigManager::spawnFallbackConfigAsString();
        }

        std::cout << "[JustReflectMe] Config wasn't found. So, fallback config spawned at: "
                     "<project_root>/"
                  << Config::jrmFolder << "/" << Config::jrmConfig << "\n";
    }

    std::string ConfigManager::spawnFallbackConfigAsString()
    {
        using namespace std::string_literals;

        auto genKey = [](const BaseConfigParam& v)
        { return "# "s + v.description.data() + "\n"s + v.paramName.data() + ": "; };

        std::string out;
        out.reserve(128);

        Config config;

        out += genKey(*config.excludedPaths) + "\n";
        for (const auto& path : config.excludedPaths->value)
        {
            out += "  - "s + path.generic_string() + "\n";
        }
        out += "\n";

        out += genKey(*config.parsableFileExtensions) + "\n";
        for (const auto& path : config.parsableFileExtensions->value)
        {
            out += "  - "s + path + "\n";
        }
        out += "\n";

        out += genKey(*config.showEveryIteratedFilePath)
               + (config.showEveryIteratedFilePath->value ? "true" : "false") + "\n\n"s;

        out += genKey(*config.showSkippedFiles)
               + (config.showSkippedFiles->value ? "true" : "false") + "\n\n"s;

        out += genKey(*config.alwaysDirtyCache)
               + (config.alwaysDirtyCache->value ? "true" : "false") + "\n\n"s;

        out += genKey(*config.insertCodeAtTheTop) + config.insertCodeAtTheTop->value + "\n\n"s;

        out += genKey(*config.insertCodeAtTheBottom) + config.insertCodeAtTheBottom->value
               + "\n\n"s;

        return out;
    }

    void ConfigManager::validateTopLevelFields(const Yaml::Node& config)
    {
        std::set<std::string> foundFields;
        for (auto it = config.Begin(); it != config.End(); it++)
        {
            auto s = (*it).first;
            if (s.empty())
            {
                break;
            }

            foundFields.emplace(std::move(s));
        }

        const Config tmp;
        for (auto&& param : tmp.getParams())
        {
            foundFields.erase(param->paramName.data());
        }

        if (foundFields.size() > 0)
        {
            std::string out;
            for (auto&& field : foundFields)
            {
                out += "'" + field + "', ";
            }
            out.pop_back();
            out.pop_back();

            std::cerr << "[JustReflectMe] Unknown fields in the config file: " << out << "\n";
        }
    }

} // namespace JRM