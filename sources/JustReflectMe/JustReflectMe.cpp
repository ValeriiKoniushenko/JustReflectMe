

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

            _projectPath = args.find(InputArgs::ProjectDir)->second;
            if (!std::filesystem::exists(_projectPath))
            {
                throw std::runtime_error("Project directory does not exist!");
            }
        }
        catch (const std::exception& er)
        {
            std::cerr << er.what() << std::endl;
            return false;
        }

        return true;
    }

    int JustReflectMe::run()
    {
        return 0;
    }

    std::unordered_map<JustReflectMe::InputArgs, std::string> JustReflectMe::parseInputArgs(
        int argc, char** argv) const
    {
        if (argc < 2)
        {
            throw std::runtime_error("Not enough arguments!");
        }

        std::unordered_map<InputArgs, std::string> out;

        out.emplace(InputArgs::ProjectDir, argv[1]);

        return out;
    }

    std::vector<JustReflectMe::FileRawData> JustReflectMe::getFilesToReflect() const
    {
        std::vector<JustReflectMe::FileRawData> out;

        return out;
    }

    void JustReflectMe::printHelp()
    {
        std::cout << "usage: JustReflectMe <project_dir>" << std::endl;
    }

} // namespace JRM