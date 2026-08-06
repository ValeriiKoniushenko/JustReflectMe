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

#include "JustReflectMe.h"

#include "Cache.h"
#include "Config.h"
#include "FileProcessor.h"
#include "Reflectors/ClassReflector.h"
#include "Reflectors/EnumClassReflector.h"
#include "version.h"

#include <filesystem>
#include <iostream>
#include <stack>
#include <unordered_map>

namespace fs = std::filesystem;

namespace JRM
{
    bool JustReflectMe::processArgs(int argc, char** argv)
    {
        if (argc < 2)
        {
            std::cerr << "[JustReflectMe] Not enough arguments!\n";
            return false;
        }

        const std::string first = argv[1];
        if (first == "--help" || first == "-h")
        {
            JustReflectMe::printHelp();
            return false;
        }

        if (first == "--version" || first == "-v")
        {
            JustReflectMe::printVersion();
            return false;
        }

        if (first == "--fallback-config" || first == "-f")
        {
            std::cout << ConfigManager::spawnFallbackConfigAsString();
            return false;
        }

        try
        {
            const auto args = JustReflectMe::parseInputArgs(argc, argv);
            if (!args.contains(InputArgs::ProjectDir))
            {
                throw std::runtime_error("Project directory is not specified!");
            }

            _sourcePath = args.find(InputArgs::ProjectDir)->second;
            if (!fs::exists(_sourcePath))
            {
                if (!_sourcePath.generic_string().empty() && _sourcePath.generic_string()[0] == '~')
                {
                    throw std::runtime_error("You can't use user-dependent '~' path: '"
                                             + _sourcePath.generic_string()
                                             + "'. Use absolute path instead.");
                }

                throw std::runtime_error("Project directory does not exist: '"
                                         + _sourcePath.generic_string() + "'");
            }
        }
        catch (const std::exception& er)
        {
            std::cerr << "[JustReflectMe] Error: " << er.what() << std::endl;
            return false;
        }

        return true;
    }

    int JustReflectMe::run(int argc, char** argv)
    {
        if (!processArgs(argc, argv))
        {
            return 1;
        }

        std::cout << "[JustReflectMe] <<< Started >>>\n";

        const auto start = std::chrono::high_resolution_clock::now();

        bool hasError = false;
        try
        {
            ConfigManager configManager;
            _config = configManager.initializeProjectAndLoadConfig(_sourcePath, hasError);

            if (_config.showEveryIteratedFilePath->value)
            {
                std::cout << "[JustReflectMe] Processing files with extensions: ";
                for (auto&& ext : _config.parsableFileExtensions->value)
                {
                    std::cout << ext << " ";
                }
                std::cout << "\n";
            }

            hasError = goThroughFiles();
        }
        catch (const std::exception& er)
        {
            std::cerr << "[JustReflectMe] Error while processing the source tree: " << er.what()
                      << std::endl;
            hasError = true;
        }

        const auto end = std::chrono::high_resolution_clock::now();
        const double duration = std::chrono::duration<double>(end - start).count();

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "[JustReflectMe] <<< Ended " << (hasError ? "with ERRORS" : "SUCCESSFULLY")
                  << " in " << duration << " sec >>>\n";

        return static_cast<int>(hasError);
    }

    std::unordered_map<JustReflectMe::InputArgs, std::string> JustReflectMe::parseInputArgs(
        int argc, char** argv)
    {
        if (!argv)
        {
            throw std::runtime_error("Invalid arguments: nullptr");
        }

        if (argc < 2)
        {
            throw std::runtime_error("Not enough arguments");
        }

        std::unordered_map<InputArgs, std::string> out;

        out.emplace(InputArgs::ProjectDir, argv[1]);

        return out;
    }

    bool JustReflectMe::isParseableFileEntry(const fs::directory_entry& entry) const
    {
        if (!entry.is_regular_file())
        {
            return false;
        }

        {
            auto path = entry.path();

            if (!path.has_extension())
            {
                return false;
            }

            path.replace_extension("");
            if (path.extension() == FileProcessor::newFileExtension)
            {
                return false;
            }
        }

        auto&& ext = entry.path().extension().generic_string();

        if (ext.empty())
        {
            return false;
        }

        for (const auto& validExt : _config.parsableFileExtensions->value)
        {
            if (ext == validExt)
            {
                return true;
            }
        }

        return false;
    }

    bool JustReflectMe::goThroughFiles()
    {
        struct Frame
        {
            fs::directory_iterator it;
            fs::path path;
        };

        Cache cache(_sourcePath, _config.alwaysDirtyCache->value);

        std::stack<Frame> frames;
        frames.push(Frame{ fs::directory_iterator(_sourcePath), _sourcePath });

        std::size_t iteratedOverTotal = 0;
        std::size_t iteratedOverParsable = 0;
        std::size_t processedTotal = 0;
        std::size_t processedWithErrors = 0;
        std::size_t processedWithWarnings = 0;

        while (!frames.empty())
        {
            Frame& currentFrame = frames.top();

            bool ignoreFramePop = false;
            for (; currentFrame.it != fs::directory_iterator(); ++currentFrame.it)
            {
                ++iteratedOverTotal;

                auto entry = *currentFrame.it;

                const auto path = entry.path();
                const auto relPath = path.lexically_relative(_sourcePath);

                if (_config.showEveryIteratedFilePath->value)
                {
                    std::cout << "[JustReflectMe] Looking at: "
                              << path.lexically_relative(_sourcePath).generic_string() << "\n";
                }

                if (entry.is_regular_file())
                {
                    if (!isParseableFileEntry(entry))
                    {
                        continue;
                    }
                }
                else if (entry.is_directory() || entry.is_symlink())
                {
                    if (_config.excludedPaths->value.contains(relPath))
                    {
                        continue;
                    }

                    ++currentFrame.it;
                    frames.push(Frame{ fs::directory_iterator(path), path });
                    ignoreFramePop = true;
                    break;
                }

                std::error_code ec;
                const auto lastWriteTime = entry.last_write_time(ec);

                if (ec)
                {
                    std::cerr << "[JustReflectMe] Error getting last write time for file: "
                              << path.lexically_relative(_sourcePath).generic_string()
                              << " Details: " << ec.message() << "\n";
                    continue;
                }

                ++iteratedOverParsable;
                if (!cache.isNeedUpdate(relPath, lastWriteTime))
                {
                    if (_config.showSkippedFiles->value)
                    {
                        std::cout << "[JustReflectMe] No need update: "
                                  << path.lexically_relative(_sourcePath).generic_string() << "\n";
                    }
                    continue;
                }

                std::cout << "[JustReflectMe] Reflecting: "
                          << path.lexically_relative(_sourcePath).generic_string() << "\n";

                ++processedTotal;
                try
                {
                    FileProcessor processor;
                    processor.registerReflector<EnumClassReflector>();
                    processor.registerReflector<ClassReflector>();

                    if (processor.run(path, _config))
                    {
                        cache.updateFile(path);
                    }

                    for (const auto& ref : processor.getReflectors())
                    {
                        processedWithWarnings += ref->numberOfWarnings();
                        processedWithErrors += ref->numberOfErrors();
                    }
                }
                catch (const std::exception& er)
                {
                    ++processedWithErrors;
                    std::cerr << "[JustReflectMe] Error while processing the source file: " << path
                              << " Message: " << er.what() << std::endl;
                }
            }

            if (!ignoreFramePop && currentFrame.it == fs::directory_iterator())
            {
                frames.pop();
            }
        }

        std::cout << "[JustReflectMe] Scanned " << iteratedOverTotal
                  << " files [parsable: " << iteratedOverParsable
                  << " | updated: " << processedTotal
                  << " | warnings/errors: " << processedWithErrors + processedWithWarnings << "]\n";

        return processedWithErrors;
    }

    void JustReflectMe::printHelp()
    {
        std::cout << "Usage: jrm <path to project>\n";
        std::cout << "\n"
                  << "Options:\n"
                  << "  --help            -h\t Print this help message.\n"
                  << "  --version         -v\t Print the version of JustReflectMe.\n"
                  << "  --fallback-config -f\t Print the fallback (default) config.\n"
                  << std::endl;
    }

    void JustReflectMe::printVersion()
    {
        std::cout << "jrm (JustReflectMe) " << APP_VERSION_MAJOR << "." << APP_VERSION_MINOR << "."
                  << APP_VERSION_PATCH << "\n\n";
        std::cout << "> Implemented by Valerii Koniushenko\n";
        std::cout << "> https://github.com/ValeriiKoniushenko/JustReflectMe\n";
    }

} // namespace JRM