

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

#include "FileProcessor.h"

#include "Config.h"
#include "FileData.h"
#include "FileNavigationHelper.h"
#include "Reflectors/BaseReflector.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace
{

    // ── Windows ──────────────────────────────────────────────────────────────────

#ifdef _WIN32

    bool ContentEquals(const std::filesystem::path& path, const std::string& text) noexcept
    {
        HANDLE h = CreateFileW(
            path.wstring().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // kernel readahead hint
            nullptr);
        if (h == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        LARGE_INTEGER sz{};
        if (!GetFileSizeEx(h, &sz) || static_cast<size_t>(sz.QuadPart) != text.size())
        {
            CloseHandle(h);
            return false;
        }
        if (text.empty())
        {
            CloseHandle(h);
            return true;
        }

        HANDLE hMap = CreateFileMappingW(h, nullptr, PAGE_READONLY, 0, 0, nullptr);
        CloseHandle(h);

        if (!hMap)
        {
            return false;
        }

        const void* view = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
        const bool equal = view && (std::memcmp(view, text.data(), text.size()) == 0);

        if (view)
        {
            UnmapViewOfFile(view);
        }
        CloseHandle(hMap);
        return equal;
    }

    void WriteContent(const std::filesystem::path& path, const std::string& text)
    {
        HANDLE h = CreateFileW(path.wstring().c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                                    "Can't open a file for write: " + path.string());
        }

        const char* ptr = text.data();
        size_t left = text.size();
        while (left > 0)
        {
            DWORD chunk = static_cast<DWORD>(std::min<size_t>(left, MAXDWORD));
            DWORD written = 0;
            if (!::WriteFile(h, ptr, chunk, &written, nullptr))
            {
                DWORD err = GetLastError();
                CloseHandle(h);
                throw std::system_error(static_cast<int>(err), std::system_category(),
                                        "Can't write to a file: " + path.string());
            }
            ptr += written;
            left -= written;
        }
        CloseHandle(h);
    }

    // ── POSIX / Linux ─────────────────────────────────────────────────────────────

#else

    bool ContentEquals(const std::filesystem::path& path, const std::string& text) noexcept
    {
        int fd = ::open(path.c_str(), O_RDONLY | O_NOATIME | O_CLOEXEC);
        if (fd == -1)
        {
            if (errno == EPERM || errno == ENOTSUP)
            {
                fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
            }
            if (fd == -1)
            {
                return false;
            }
        }

        struct stat st{};
        if (::fstat(fd, &st) == -1 || static_cast<size_t>(st.st_size) != text.size())
        {
            ::close(fd);
            return false;
        }
        if (text.empty())
        {
            ::close(fd);
            return true;
        }

        ::posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
        ::posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);

        const auto sz = static_cast<size_t>(st.st_size);
        void* map = ::mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0);
        ::close(fd);

        if (map == MAP_FAILED)
        {
            return false;
        }

        ::madvise(map, sz, MADV_SEQUENTIAL);

        const bool equal = (std::memcmp(map, text.data(), sz) == 0);
        ::munmap(map, sz);
        return equal;
    }

    void WriteContent(const std::filesystem::path& path, const std::string& text)
    {
        int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
        if (fd == -1)
        {
            throw std::system_error(errno, std::system_category(), "open: " + path.string());
        }

        const char* ptr = text.data();
        size_t left = text.size();
        while (left > 0)
        {
            ssize_t n = ::write(fd, ptr, left);
            if (n < 0)
            {
                int err = errno;
                ::close(fd);
                throw std::system_error(err, std::system_category(), "write: " + path.string());
            }
            ptr += n;
            left -= static_cast<size_t>(n);
        }

        if (::close(fd) == -1)
        {
            throw std::system_error(errno, std::system_category(), "close: " + path.string());
        }
    }

#endif

} // anonymous namespace

namespace
{
    [[nodiscard]] std::string ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Cannot open a file: " + filename);
        }

        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer;
        buffer.reserve(size);
        buffer.resize(size);

        if (!file.read(buffer.data(), size)) [[unlikely]]
        {
            throw std::runtime_error("Error reading file");
        }

        return buffer;
    }
} // namespace

namespace JRM
{

    const std::vector<std::unique_ptr<BaseReflector>>& FileProcessor::getReflectors() const
    {
        return _reflectors;
    }

    const std::string& FileProcessor::getHeaderFilename() const
    {
        return _path;
    }

    std::string FileProcessor::getSourceFilename() const
    {
        std::filesystem::path path(_path);
        if (path.has_extension()) [[likely]]
        {
            path.replace_extension(".cpp");
            return path.generic_string();
        }
        return {};
    }

    bool FileProcessor::isGeneratedFilename(const std::string& filename)
    {
        std::filesystem::path path(filename);
        if (path.has_extension()) [[likely]]
        {
            path.replace_extension("");
            if (path.has_extension())
            {
                return path.extension().generic_string() == newFileExtension;
            }
        }

        return false;
    }

    std::set<std::string> FileProcessor::getAllRequiredIncludes() const
    {
        std::set<std::string> out;
        for (const auto& reflector : _reflectors)
        {
            if (reflector->hasTokens())
            {
                auto tmp = reflector->getIncludes();
                out.insert(tmp.begin(), tmp.end());
            }
        }
        return out;
    }

    void FileProcessor::WriteIfDifferent(const std::string& text, const std::filesystem::path& path)
    {
        if (const auto parent = path.parent_path(); !parent.empty())
        {
            std::error_code ec;
            if (!fs::exists(parent, ec) || ec)
            {
                throw std::runtime_error("Parent directory does not exist: " + parent.string());
            }
        }

        std::error_code ec;
        if (fs::exists(path, ec) && !ec)
        {
            if (!fs::is_regular_file(path))
            {
                throw std::runtime_error("Not a regular file: " + path.string());
            }
            if (ContentEquals(path, text))
            {
                return; // identical - timestamps are untouched
            }
        }

        WriteContent(path, text);
    }

    void FileProcessor::scanContent(FileData& data) const
    {
        for (std::size_t i = 0; i < _reflectors.size(); ++i)
        {
            _reflectors[i]->scanContent(data);
        }
    }

    PostProcessedFile FileProcessor::getFileContent(const std::string& filename)
    {
        PostProcessedFile out;

        std::string content1 = ReadFile(filename);

        std::string content2;
        content2.reserve(content1.size());

        // Removing all block comments
        bool nowInsideBlockComment = false;
        for (std::size_t i = 0; i < content1.size() && content1[i]; ++i)
        {
            if (i > 0 && content1[i - 1] == '/' && content1[i] == '*')
            {
                if (nowInsideBlockComment) [[unlikely]]
                {
                    throw std::runtime_error(
                        "Can't parse a file: " + filename
                        + " - troubles with parsing block comment. File can be incorrect.");
                }
                nowInsideBlockComment = true;
                if (!content2.empty())
                {
                    content2.pop_back();
                }
            }

            if (i > 0 && content1[i - 1] == '*' && content1[i] == '/')
            {
                nowInsideBlockComment = false;
                continue;
            }

            if (!nowInsideBlockComment || FileNavigator::IsNewLine(content1[i]))
            {
                content2.push_back(content1[i]);
            }
        }
        content1.resize(0);

        // Removing all comments: //
        bool nowComment = false;
        for (std::size_t i = 0; i < content2.size() && content2[i]; ++i)
        {
            if (nowComment)
            {
                if (FileNavigator::IsNewLine(content2[i]))
                {
                    nowComment = false;
                }
                else
                {
                    continue;
                }
            }

            if (i > 0 && content2[i] == '/' && content2[i - 1] == '/')
            {
                nowComment = true;
                content1.pop_back();
                continue;
            }

            content1.push_back(content2[i]);
        }
        content2.resize(0);

        // Removing all strings: "hello world"
        bool nowDoubleQuote = false;
        std::pair<std::size_t, std::string> stringToken;

        for (std::size_t i = 0; i < content1.size() && content1[i]; ++i)
        {
            if (nowDoubleQuote && FileNavigator::IsNewLine(content1[i]))
            {
                nowDoubleQuote = false;
            }

            if (content1[i] == '"')
            {
                nowDoubleQuote = !nowDoubleQuote;
                if (nowDoubleQuote && (i > 0 && content1[i - 1] != '\\'))
                {
                    if (stringToken.first != 0)
                    {
                        content2.push_back(PostProcessedFile::stringPlaceholder);
                        out.stringTokens.insert(std::move(stringToken));
                    }

                    stringToken = { i, "" };
                }

                if (!nowDoubleQuote)
                {
                    continue;
                }
            }

            if (nowDoubleQuote)
            {
                content2.push_back(PostProcessedFile::stringPlaceholder);

                if (i != stringToken.first)
                {
                    stringToken.second.push_back(content1[i]);
                }
            }
            else
            {
                content2.push_back(content1[i]);
            }
        }
        content1.resize(0);
        if (stringToken.first != 0)
        {
            out.stringTokens.insert(std::move(stringToken));
        }

        // Removing all chars: 'c'
        bool nowSingleQuote = false;
        std::pair<std::size_t, std::string> charToken;

        for (std::size_t i = 0; i < content2.size() && content2[i]; ++i)
        {
            bool hasNumBefore = i > 0 ? std::isdigit(content2[i - 1]) : false;
            bool hasNumAfter = (i + 1) < content2.size() ? std::isdigit(content2[i + 1]) : false;

            if (content2[i] == '\'' && !(hasNumBefore && hasNumAfter))
            {
                nowSingleQuote = !nowSingleQuote;
                if (nowSingleQuote && (i > 0 && content2[i - 1] != '\\'))
                {
                    if (charToken.first != 0)
                    {
                        content1.push_back(PostProcessedFile::charPlaceholder);
                        out.charTokens.insert(std::move(charToken));
                    }

                    content1.push_back(PostProcessedFile::charPlaceholder);
                    charToken = { i, "" };
                }

                if (!nowSingleQuote)
                {
                    continue;
                }
            }

            if (nowSingleQuote)
            {
                content1.push_back(PostProcessedFile::charPlaceholder);

                if (i != charToken.first)
                {
                    charToken.second.push_back(content2[i]);
                }
            }
            else
            {
                content1.push_back(content2[i]);
            }
        }
        content2.resize(0);
        if (charToken.first != 0)
        {
            out.charTokens.insert(std::move(charToken));
        }

        // normalizing enum<x spaces>class -> enum<one space>class
        if (const auto pos = content1.find("enum  "); pos != std::string::npos)
        {
            constexpr std::string_view enumStr = "enum";
            for (std::size_t i = 0; i < content1.size() - enumStr.size() && content1[i]; ++i)
            {
                if (strncmp(content1.data() + i, enumStr.data(), enumStr.size()) == 0)
                {
                    content2.append("enum ");
                    i += enumStr.size() + 1;

                    while (i < content1.size() && content1[i] == ' ')
                    {
                        ++i;
                    }
                    --i;
                }
                else
                {
                    content2.push_back(content1[i]);
                }
            }
            content1.resize(0);
        }

        // normalizing "class Foo : public Bar1,\npublic Bar2,\npublic Bar3" to one line
        {
            auto& content = content1.empty() ? content2 : content1;

            constexpr std::string_view classStr = "class";
            std::size_t offset = 0;
            std::size_t i = 0;
            while ((i = content.find(classStr.data(), offset)) != std::string::npos)
            {
                // Verifying that we found the exact a class
                const char* classStart = content.data() + i;
                const char* tmp = classStart;
                if ((classStart = FileNavigator::GoToSpace(classStart)) == tmp)
                {
                    offset = i + 1;
                    continue;
                }
                tmp = classStart;

                if ((classStart = FileNavigator::GoToNotSpace(classStart)) == tmp
                    || (!std::isalpha(*classStart) && *classStart != '_'))
                {
                    offset = i + 1;
                    continue;
                }
                tmp = classStart;

                while (*classStart
                       && (std::isalpha(*classStart) || *classStart == '_' || *classStart == ' '))
                {
                    ++classStart;
                }

                if (*classStart == '<')
                {
                    classStart = FileNavigator::FindScopeEnd(classStart);
                    if (!classStart)
                    {
                        offset = i + 1;
                        continue;
                    }
                }
                classStart = FileNavigator::GoToNotSpace(classStart);

                if (FileNavigator::StartWith(classStart, "final"))
                {
                    classStart = FileNavigator::GoToNotSpace(classStart + 5);
                }

                if (*classStart != ':' && *classStart != ';' && *classStart != '{')
                {
                    offset = i + 1;
                    continue;
                }

                auto end = content.find_first_of(";{", i);

                for (auto j = i; j < end; ++j)
                {
                    if (FileNavigator::IsNewLine(content[j]))
                    {
                        content[j] = PostProcessedFile::newLinePlaceholder;
                    }
                }

                offset = i + 1;
            }
        }

        out.content = std::move(content1.empty() ? content2 : content1);
        return out;
    }

    std::pair<std::string, std::string> FileProcessor::generateFilenames(
        bool onlyFileNames /* = false*/) const
    {
        const auto extIndex = _path.find_last_of(".");
        if (extIndex == std::string::npos)
        {
            throw std::runtime_error("Can't find extension in file name: " + _path);
        }

        std::string headerPath = _path.substr(0, extIndex);
        headerPath += newFileExtension;
        std::string sourcePath;
        if (!_pathImpl.empty())
        {
            sourcePath = headerPath;
            headerPath += ".h";
            sourcePath += ".cpp.inl";
        }
        else
        {
            headerPath += ".inl";
        }

        if (onlyFileNames)
        {
            namespace fs = std::filesystem;

            std::filesystem::path path = _path;

            if (path.has_parent_path())
            {
                headerPath = fs::relative(headerPath, path.parent_path().generic_string())
                                 .generic_string();

                sourcePath = fs::relative(sourcePath, path.parent_path().generic_string())
                                 .generic_string();
            }
        }

        return { headerPath, sourcePath };
    }

    bool FileProcessor::hasAtLeastOneToken() const
    {
        for (const auto& reflector : _reflectors)
        {
            if (reflector->hasTokens())
            {
                return true;
            }
        }
        return false;
    }

    bool FileProcessor::tryToGenerateHeaderContent(FileData& data)
    {
        bool errors = false;

        std::string result;
        result.reserve(1024);

        // HEADER
        result += warningCommentAtFileTop;
        result += "\n\n";
        result += _config->insertCodeAtTheTop->value;
        if (!result.empty() && result.back() != '\n')
        {
            result += '\n';
        }

        // INCLUDES
        for (auto&& include : getAllRequiredIncludes())
        {
            result += "#include <" + include + ">\n";
        }
        result += "\n";

        bool atLeastOneInsert = false;
        for (const auto& reflector : _reflectors)
        {
            if (!reflector->hasTokens())
            {
                continue;
            }

            atLeastOneInsert = true;
            result += reflector->generateHeaderFile(data);
            errors |= reflector->hasErrors();
        }

        result += _config->insertCodeAtTheBottom->value;
        if (!result.empty() && result.back() != '\n')
        {
            result += '\n';
        }

        if (!atLeastOneInsert)
        {
            return false;
        }

        WriteIfDifferent(result, generateFilenames().first);
        return !errors;
    }

    bool FileProcessor::tryToGenerateSourceContent(FileData& data)
    {
        bool errors = false;

        const auto cppPath = generateFilenames().second;
        if (cppPath.empty()) [[unlikely]]
        {
            return true;
        }

        std::string src;
        src += warningCommentAtFileTop;
        src += "\n\n";
        src += _config->insertCodeAtTheTop->value;
        if (!src.empty() && src.back() != '\n')
        {
            src += '\n';
        }

        bool atLeastOneInsert = false;

        for (const auto& reflector : _reflectors)
        {
            if (!reflector->hasTokens() || !reflector->hasImplTranslationUnit())
            {
                continue;
            }

            atLeastOneInsert = true;
            src += reflector->generateSourceFile(generateFilenames(true).first, data);
            errors |= reflector->hasErrors();
        }

        if (!atLeastOneInsert)
        {
            return true;
        }

        src += _config->insertCodeAtTheBottom->value;
        if (!src.empty() && src.back() != '\n')
        {
            src += '\n';
        }

        WriteIfDifferent(src, cppPath);
        return !errors;
    }

    bool FileProcessor::tryToIntegrateIncludes(const FileData& data)
    {
        const auto [generatedHpp, generatedCpp] = generateFilenames(true);

        integrateHeaderIncludes(data, generatedHpp);
        integrateSourceIncludes(data, generatedCpp);

        return true;
    }

    void FileProcessor::integrateHeaderIncludes(const FileData& data,
                                                const std::string& generatedHpp)
    {
        const auto includeString = "#include \"" + generatedHpp + "\"";
        const auto hpp = getHeaderFilename();
        auto originalSources = ReadFile(hpp);

        if (originalSources.find(includeString) != std::string::npos)
        {
            return;
        }

        std::size_t numberOfEndlines = 0;
        {
            std::size_t i = std::min(originalSources.size() - 1, originalSources.size());
            while (std::isspace(originalSources.at(i)))
            {
                if (originalSources.at(i) == '\n')
                {
                    ++numberOfEndlines;
                }

                --i;
            }
        }

        std::string integrationString;
        for (auto i = numberOfEndlines; i < 2; ++i)
        {
            integrationString += "\n";
        }

        integrationString += includeString;
        integrationString += " // added by the code generator. Better don't move it.\n";

        originalSources += integrationString;

        std::ofstream out(hpp);
        if (!out.is_open())
        {
            throw std::runtime_error(
                "Cannot open file for write, to integrate generated #include-s: " + hpp);
        }
        out.write(originalSources.c_str(), originalSources.size() * sizeof(char));

        onPostGenerateHeaderContent(originalSources);
    }

    void FileProcessor::integrateSourceIncludes(const FileData& data,
                                                const std::string& generatedCpp)
    {
        if (generatedCpp.empty() || _pathImpl.empty())
        {
            return;
        }

        if (!std::ranges::any_of(_reflectors, [](const auto& reflector)
                                 { return reflector->hasImplTranslationUnit(); }))
        {
            return;
        }

        const auto includeString = "#include \"" + generatedCpp + "\"";
        auto originalSources = ReadFile(_pathImpl);
        if (originalSources.contains(includeString))
        {
            return;
        }

        std::string integrationString;
        std::size_t numberOfEndlines = 0;
        {
            std::size_t i = std::min(originalSources.size() - 1, originalSources.size());
            while (std::isspace(originalSources.at(i)))
            {
                if (originalSources.at(i) == '\n')
                {
                    ++numberOfEndlines;
                }
            }
        }

        for (auto i = numberOfEndlines; i < 2; ++i)
        {
            integrationString += "\n";
        }

        integrationString += "\n";
        integrationString += includeString;
        integrationString += " // this line added by the code generator.\n";

        // find the last #include and put it under it
        auto index = originalSources.rfind("#include");
        if (index == std::string::npos)
        {
            index = 0;
        }
        else
        {
            auto found
                = originalSources.find_first_of("\n\x1D", index); // \x1D is newLinePlaceholder
            if (found != std::string::npos)
            {
                index = found + 1;
            }
            else
            {
                index = originalSources.size();
            }
        }
        originalSources.insert(index, integrationString);

        std::ofstream out(_pathImpl);
        if (!out.is_open())
        {
            throw std::runtime_error(
                "Cannot open file for write, to integrate generated #include-s: " + generatedCpp);
        }
        out.write(originalSources.c_str(), originalSources.size() * sizeof(char));
    }

    std::string FileProcessor::extrudeImplPath(std::filesystem::path path)
    {
        if (!path.has_extension()) [[unlikely]]
        {
            return {};
        }
        path.replace_extension("");
        const auto strPath = path.generic_string();

        // TODO: optimize it! Super slow.
        static const std::array extensions
            = { std::string(".cpp"), std::string(".cxx"), std::string(".cc") };

        const auto found = std::ranges::find_if(extensions, [&](const auto& ext)
                                                { return std::filesystem::exists(strPath + ext); });

        if (found == extensions.end())
        {
            return {};
        }

        return strPath + *found;
    }

    bool FileProcessor::generateNewContent(FileData& data)
    {
        bool isOk = true;

        if (hasAtLeastOneToken())
        {
            isOk &= tryToGenerateHeaderContent(data);
            isOk &= tryToGenerateSourceContent(data);
            isOk &= tryToIntegrateIncludes(data);
        }

        return isOk;
    }

    bool FileProcessor::run(const std::filesystem::path& path, const Config& config)
    {
        bool isOk = false;

        _config = &config;
        _path = path.generic_string();

        if (!std::filesystem::exists(_path))
        {
            throw std::runtime_error("File does not exist: '" + _path + "'");
        }

        _pathImpl = extrudeImplPath(path);
        for (const auto& reflector : _reflectors)
        {
            reflector->setHasImplTranslationUnit(!_pathImpl.empty());
            reflector->setConfig(config);
        }

        FileData data;
        try
        {
            data.setContent(FileProcessor::getFileContent(_path));
            data.setPath(_path);

            onPreGenerateContent(data.getContent());

            scanContent(data);
            isOk = generateNewContent(data);
        }
        catch (const JRM::SyntaxException& e)
        {
            std::cerr << "[JustReflectMe] " << e.getFullMessage(data.getContent(), _path) << "\n";
            isOk = false;
        }
        catch (const JRM::GenerationException& e)
        {
            isOk = false;
            std::cerr << "[JustReflectMe] " << _path << ": generation exception: " << e.what()
                      << "\n";
        }
        catch (const std::exception& e)
        {
            isOk = false;
            std::cerr << "[JustReflectMe] Error while processing the file: '" << _path
                      << "' Details: " << e.what() << "\n";
        }
        return isOk;
    }

} // namespace JRM