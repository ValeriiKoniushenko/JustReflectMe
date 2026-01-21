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

namespace JRM
{

    Config ConfigManager::initializeProjectAndLoadConfig(const std::filesystem::path& projectDir)
    {
        _projectDir = projectDir;

        Config config;

        try
        {
            Yaml::Node yaml;
            if (!std::filesystem::exists(_projectDir / jrmFolder)
                || !std::filesystem::exists(_projectDir / jrmFolder / jrmConfig))
            {
                spawnFallbackFileConfig();
                Yaml::Parse(yaml, spawnFallbackConfigAsString());
            }
            else
            {
                Yaml::Parse(yaml, (_projectDir / jrmFolder / jrmConfig).generic_string().c_str());
            }


            if (!yaml[Config::propName_excludedPaths].IsNone())
            {
                auto item = yaml[Config::propName_excludedPaths];
                for(auto it = item.Begin(); it != item.End(); it++)
                {
                    config.excludedPaths.emplace((*it).second.As<std::string>());
                }
            }
        }
        catch (Yaml::Exception& ex)
        {
            std::cerr << "Failed to parse .yaml config file. Details: " << ex.what() << std::endl;
            std::cerr << "The JRM is configured incompletely!" << std::endl;
        }

        return config;
    }

    void ConfigManager::spawnFallbackFileConfig()
    {
        std::error_code ec;
        if (!std::filesystem::create_directory(_projectDir / jrmFolder, ec))
        {
            std::cerr << "Failed to create jrm folder at the project's root. Details: "
                      << ec.message() << std::endl;
            return;
        }

        {
            std::ofstream out(_projectDir / jrmFolder / jrmConfig);
            if (!out.is_open())
            {
                std::cerr << "Failed to spawn a jrm fallback config at: <project_root>/"
                          << jrmFolder << "/" << jrmConfig << std::endl;
                return;
            }
            out << spawnFallbackConfigAsString();
        }

        std::cout << "Config wasn't found. So, fallback config spawned at: <project_root>/" << jrmFolder << "/" << jrmConfig << std::endl;
    }

    std::string ConfigManager::spawnFallbackConfigAsString()
    {
        using namespace std::string_literals;

        std::string out;
        out.reserve(128);

        out += Config::propName_excludedPaths + ":\n"s;
        out += " - build\n";
        out += " - .vscode\n";
        out += " - .cache\n";
        out += " - .git\n";
        out += " - .idea\n";
        out += " - "s + jrmFolder + "\n";

        return out;
    }

} // namespace JRM