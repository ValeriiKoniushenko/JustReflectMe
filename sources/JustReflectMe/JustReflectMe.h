// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

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
