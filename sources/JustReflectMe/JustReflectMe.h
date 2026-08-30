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

#include "Config.h"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace JRM
{

    /**
     * @brief Command-line entry point for scanning and generating a project.
     *
     * The expected invocation is `jrm <path-to-project>`. The project configuration is read from
     * `Config::jrmFolder/Config::jrmConfig`, and eligible source files are processed with the
     * built-in reflectors.
     */
    class JustReflectMe final
    {
    public:
        JustReflectMe() = default;
        JustReflectMe(const JustReflectMe&) = delete;
        JustReflectMe& operator=(const JustReflectMe&) = delete;
        JustReflectMe(JustReflectMe&&) noexcept = delete;
        JustReflectMe& operator=(JustReflectMe&&) noexcept = delete;
        ~JustReflectMe() = default;

        /**
         * @brief Runs JustReflectMe with command-line arguments.
         * @param argc Number of command-line arguments.
         * @param argv Command-line argument values.
         * @return `0` after a successful run and a non-zero value when argument parsing or file
         * processing fails.
         */
        [[nodiscard]] int run(int argc, char** argv);

    private:
        enum class InputArgs
        {
            ProjectDir
        };

    private:
        [[nodiscard]] bool processArgs(int argc, char** argv);
        static void printHelp();
        static void printVersion();
        [[nodiscard]] static std::unordered_map<InputArgs, std::string> parseInputArgs(int argc,
                                                                                       char** argv);
        [[nodiscard]] bool isParseableFileEntry(
            const std::filesystem::directory_entry& entry) const;
        [[nodiscard]] bool goThroughFiles();

    private:
        std::filesystem::path _sourcePath;
        Config _config;
    };

} // namespace JRM
