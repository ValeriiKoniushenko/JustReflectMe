

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

#include "Reflectors/BaseReflector.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <typeindex>
#include <unordered_set>
#include <vector>

#if defined(JRM_ENABLE_TESTS)
    #include "gtest/gtest_prod.h"
#endif

namespace JRM
{
    class FileData;
    struct Config;

    struct PostProcessedFile
    {
        constexpr static char stringPlaceholder = 31;
        constexpr static char charPlaceholder = 30;
        constexpr static char newLinePlaceholder = 29;

        std::string content;
        std::unordered_map<std::size_t, std::string> stringTokens;
        std::unordered_map<std::size_t, std::string> charTokens;
    };

    class FileProcessor
    {
    public:
        static constexpr const char* warningCommentAtFileTop = R"(/*
 * This code was generated automatically with
 * https://github.com/ValeriiKoniushenko/JustReflectMe
 *
 * DO NOT EDIT MANUALLY!
 * Your changes will be replaced next time
 */)";

        inline static const char* newFileExtension = ".generated";

        FileProcessor() = default;
        FileProcessor(const FileProcessor&) = delete;
        FileProcessor& operator=(const FileProcessor&) = delete;
        FileProcessor(FileProcessor&&) noexcept = delete;
        FileProcessor& operator=(FileProcessor&&) noexcept = delete;
        virtual ~FileProcessor() = default;

        [[nodiscard]] bool generateNewContent(FileData& data);
        [[nodiscard]] bool run(const std::filesystem::path& path, const Config& config);

        template<IsBaseReflector T>
        void registerReflector();

        template<IsBaseReflector T>
        [[nodiscard]] bool hasReflector();

        [[nodiscard]] const std::string& getHeaderFilename() const;
        [[nodiscard]] std::string getSourceFilename() const;
        [[nodiscard]] static bool isGeneratedFilename(const std::string& filename);

    protected:
        virtual void onPreGenerateContent(const std::string& content) const {}
        virtual void onPostGenerateHeaderContent(const std::string& content) const {}
        [[nodiscard]] std::set<std::string> getAllRequiredIncludes() const;

        /** Writes `text` to `path` only if the existing content differs.
         *  No write -> timestamps stay unchanged.
         *
         *  Linux:   mmap + O_NOATIME + posix_fadvise for zero-copy comparison
         *  Windows: CreateFileMapping + FILE_FLAG_SEQUENTIAL_SCAN
         *
         *  @throw std::system_error  on I/O failure
         *  @throw std::runtime_error on a bad path
         */
        void WriteIfDifferent(const std::string& text, const std::filesystem::path& path);

    private:
        void scanContent(FileData& data) const;
        [[nodiscard]] static PostProcessedFile getFileContent(const std::string& filename);
        [[nodiscard]] std::pair<std::string, std::string> generateFilenames(bool onlyFileNames
                                                                            = false) const;
        [[nodiscard]] bool hasAtLeastOneToken() const;
        [[nodiscard]] bool tryToGenerateHeaderContent(FileData& data);
        [[nodiscard]] bool tryToGenerateSourceContent(FileData& data);
        [[nodiscard]] bool tryToIntegrateIncludes(const FileData& data);

        void integrateHeaderIncludes(const FileData& data, const std::string& generatedHpp);
        void integrateSourceIncludes(const FileData& data, const std::string& generatedCpp);

        [[nodiscard]] static std::string extrudeImplPath(std::filesystem::path path);

    protected:
        std::vector<std::unique_ptr<BaseReflector>> _reflectors;
        std::unordered_set<std::type_index> _reflectorsMeta;

        std::string _path;     // to .h   - must be filled
        std::string _pathImpl; // to .cpp - can be empty
        const Config* _config = nullptr;

#if defined(JRM_ENABLE_TESTS)
        FRIEND_TEST(FileProcessorTests, FindAllEntryPoints);
#endif
    };

    // =====================================================
    //                   IMPLEMENTATIONS
    // =====================================================
    template<IsBaseReflector T>
    void FileProcessor::registerReflector()
    {
#if defined(NDEBUG)
        if (hasReflector<T>())
        {
            std::cerr << "[JustReflectMe] Such a reflector '" << typeid(T).name()
                      << "' already registered!\n";
            return;
        }
#endif

        _reflectors.emplace_back(std::make_unique<T>());
        _reflectorsMeta.emplace(std::type_index(typeid(T)));
    }

    template<IsBaseReflector T>
    bool FileProcessor::hasReflector()
    {
        return _reflectorsMeta.contains(std::type_index(typeid(T)));
    }

} // namespace JRM
