

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

#include "Reflectors/BaseReflector.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#if defined(JRM_ENABLE_TESTS)
    #include "gtest/gtest_prod.h"
#endif

namespace JRM
{
    class FileData;

    class FileProcessor
    {
    public:
        inline static const char* newFileExtension = ".generated";

        FileProcessor() = default;
        FileProcessor(const FileProcessor&) = delete;
        FileProcessor& operator=(const FileProcessor&) = delete;
        FileProcessor(FileProcessor&&) noexcept = delete;
        FileProcessor& operator=(FileProcessor&&) noexcept = delete;
        virtual ~FileProcessor() = default;

        void generateNewContent(FileData& data);
        void run(const std::filesystem::path& path);

        template<IsBaseReflector T>
        void registerReflector();

        template<IsBaseReflector T>
        [[nodiscard]] bool hasReflector();

        [[nodiscard]] std::string getHeaderFilename() const;
        [[nodiscard]] std::string getSourceFilename() const;
        [[nodiscard]] static bool isGeneratedFilename(const std::string& filename);

    protected:
        virtual void onPreGenerateContent(const std::string& content) const {}
        virtual void onPostGenerateHeaderContent(const std::string& content) const {}

    private:
        void scanContent(FileData& data) const;
        [[nodiscard]] std::string getFileContent(const std::string& filename) const;
        [[nodiscard]] std::pair<std::string, std::string> generateFilenames(
            const BaseReflector* reflector, bool onlyFileNames = false) const;
        void tryToGenerateHeaderContent(const BaseReflector* reflector, FileData& data);
        void tryToGenerateSourceContent(const BaseReflector* reflector, FileData& data);
        void tryToIntegrateIncludes(const BaseReflector* reflector, FileData& data);


        void integrateHeaderIncludes(const BaseReflector* reflector, FileData& data, const std::string& generatedHpp);
        void integrateSourceIncludes(const BaseReflector* reflector, FileData& data, const std::string& generatedCpp);


        [[nodiscard]] static std::string extrudeImplPath(std::filesystem::path path);

    protected:
        std::vector<std::unique_ptr<BaseReflector>> _reflectors;
        std::string _path;     // to .h   - must be filled
        std::string _pathImpl; // to .cpp - can be empty

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
            std::cerr << "[JustReflectMe] Such a reflector '" << typeid(T).name() << "' already registered!\n";
            return;
        }
#endif

        _reflectors.emplace_back(std::make_unique<T>());
    }

    template<IsBaseReflector T>
    bool FileProcessor::hasReflector()
    {
        for (auto&& reflector : _reflectors)
        {
            if (typeid(*reflector) == typeid(T))
            {
                return true;
            }
        }

        return false;
    }

} // namespace JRM
