

/*
 * MIT License
 *
 * Copyright (c) 2018-2025 Valerii Koniushenko
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

#include "FileProcessor.h"

#include <filesystem>
#include <iostream>
#include <unordered_map>

namespace JRM
{
    bool JustReflectMe::setArgs(int argc, char** argv)
    {
        if (argc < 2)
        {
            std::cerr << "Not enough arguments!" << std::endl;
            return false;
        }

        try
        {
            const auto args = parseInputArgs(argc, argv);
            if (args.find(InputArgs::ProjectDir) == args.end())
            {
                throw std::runtime_error("Project directory is not specified!");
            }

            _sourcePath = args.find(InputArgs::ProjectDir)->second;
            if (!std::filesystem::exists(_sourcePath))
            {
                throw std::runtime_error("Project directory does not exist!");
            }
        }
        catch (const std::exception& er)
        {
            std::cerr << "Error: " << er.what() << std::endl;
            return false;
        }

        return true;
    }

    int JustReflectMe::run()
    {
        std::cout << "Finding all files with extensions: ";
        for (auto&& ext : _parseableFileExtensions)
        {
            std::cout << ext << " ";
        }
        std::cout << "\n";

        try
        {
            for (auto&& entry : std::filesystem::recursive_directory_iterator(_sourcePath))
            {
                if (!isParseableEntry(entry))
                {
                    continue;
                }

                auto&& path = entry.path().generic_string();
                std::cout << "Processing: " << path << "\n";

                try
                {
                    FileProcessor processor;
                    processor.setFilePath(path);
                    processor.run();
                }
                catch (const std::exception& er)
                {
                    std::cerr << "Error while processing the source file: " << path
                              << " Message: " << er.what() << std::endl;
                }
            }
        }
        catch (const std::exception& er)
        {
            std::cerr << "Error while processing the source tree: " << er.what() << std::endl;
        }

        return 0;
    }

    std::unordered_map<JustReflectMe::InputArgs, std::string> JustReflectMe::parseInputArgs(
        int argc, char** argv) const
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

    bool JustReflectMe::isParseableEntry(const std::filesystem::directory_entry& entry) const
    {
        if (!entry.is_regular_file())
        {
            return false;
        }

        auto&& ext = entry.path().extension().generic_string();

        if (ext.empty())
        {
            return false;
        }

        for (const auto& validExt : _parseableFileExtensions)
        {
            if (ext == validExt)
            {
                return true;
            }
        }

        return false;
    }

    void JustReflectMe::printHelp()
    {
        std::cout << "usage: jrm <source_path>" << std::endl;
    }

} // namespace JRM