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

#include "JustReflectMe/Enums.h"

#include <concepts>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace JRM
{
    class FileProcessor;
    struct Config;
    struct Scope;
    class FileData;

    /**
     * @brief Metadata describing a reflected type.
     */
    struct TypeMeta
    {
        ContextType type = ContextType::Undefined;
    };

    /**
     * @brief Reports a syntax error together with its position in a source file.
     */
    class SyntaxException : public std::runtime_error
    {
    public:
        /**
         * @param message Error description.
         * @param indexInFile Zero-based character position of the error.
         */
        SyntaxException(const std::string& message, std::size_t indexInFile);

        /**
         * @brief Formats the error with source path, line, and column information.
         * @param content Source content containing the error.
         * @param pathToFile Path displayed in the formatted message.
         * @return A diagnostic in `path:line:column: error: message` form.
         */
        [[nodiscard]] std::string getFullMessage(const std::string& content,
                                                 const std::string& pathToFile) const;

    protected:
        std::size_t _indexInFile = std::numeric_limits<std::size_t>::max();
    };

    /**
     * @brief Reports an error while generating reflection code.
     */
    class GenerationException : public std::runtime_error
    {
    public:
        explicit GenerationException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /**
     * @brief Common interface and workflow for a reflection marker processor.
     *
     * A reflector recognizes a trigger keyword, scans the corresponding file tokens, and emits
     * generated header/source fragments. Concrete reflectors implement the language-specific scan
     * and generation steps.
     */
    class BaseReflector
    {
    public:
        constexpr static std::string_view namespaceName = "R";

        enum class Severity
        {
            Warning,
            Error
        };

        /**
         * @brief Converts a diagnostic severity to its display name.
         * @param severity Severity to convert.
         * @return `"warning"`, `"error"`, or `"unknown"`.
         */
        [[nodiscard]] static std::string SeverityToString(Severity severity);

    public:
        BaseReflector() = default;
        BaseReflector(const BaseReflector&) = default;
        BaseReflector& operator=(const BaseReflector&) = default;
        BaseReflector(BaseReflector&&) noexcept = default;
        BaseReflector& operator=(BaseReflector&&) noexcept = default;
        virtual ~BaseReflector() = default;

        /**
         * @brief Checks whether the content contains this reflector's trigger keyword.
         * @param content Preprocessed source content.
         * @return `true` when a valid trigger occurrence is present.
         */
        [[nodiscard]] bool canProcessContent(const std::string& content) const;

        /**
         * @brief Finds trigger tokens and invokes the concrete reflector scan.
         * @param data File data to inspect and enrich with reflection state.
         */
        void scanContent(FileData& data);

        /**
         * @brief Tests whether scanning found at least one trigger token.
         */
        [[nodiscard]] bool hasTokens() const noexcept { return !_tokens.empty(); }

        [[nodiscard]] virtual constexpr std::string_view getTriggerKeyword() const noexcept = 0;
        [[nodiscard]] virtual std::optional<TypeMeta> findKnownTypeMeta(
            const std::string& fullPath) const = 0;
        [[nodiscard]] std::optional<TypeMeta> findGloballyKnownTypeMeta(
            const std::string& fullPath) const;

        /**
         * @brief Generates this reflector's header fragment.
         * @param data File data used during generation.
         * @return Generated C++ header text.
         */
        [[nodiscard]] std::string generateHeaderFile(FileData& data) const;

        /**
         * @brief Generates this reflector's source fragment.
         * @param newHeaderPath Relative path to the generated header.
         * @param data File data used during generation.
         * @return Generated C++ source text.
         */
        [[nodiscard]] std::string generateSourceFile(const std::string& newHeaderPath,
                                                     FileData& data) const;

        /**
         * @brief Returns additional standard-library headers required by generated code.
         * @return An empty set by default.
         */
        [[nodiscard]] virtual std::set<std::string> getIncludes() const;

        void setHasImplTranslationUnit(bool val) noexcept;
        [[nodiscard]] bool hasImplTranslationUnit() const noexcept;
        [[nodiscard]] bool isSupportImplTranslationUnit() const noexcept;
        void setParentFileProcessor(FileProcessor* parent) noexcept;

        [[nodiscard]] bool hasWarnings() const;
        [[nodiscard]] bool hasErrors() const;
        [[nodiscard]] int numberOfWarnings() const;
        [[nodiscard]] int numberOfErrors() const;
        /**
         * @brief Returns the number of diagnostics recorded for a severity.
         * @param severity Severity to count.
         * @return Number of matching diagnostics.
         */
        [[nodiscard]] int numberOfSeverity(Severity severity) const;

        void setConfig(const Config& config) noexcept;

        virtual void postScanCrossLinksResolving() = 0;

    protected:
        /**
         * Name mark - full name with namespaces & all nesting.
         * @code
         * namespace Foo {
         *     class Bar{
         *         struct Hello{};
         *     };
         * }
         * @endcode
         * In such a case the name mark for Hello is 'Foo::Bar::Hello'
         */
        static constexpr std::string_view nameMark = "@@NAME_";

        /**
         * Parent mark - full path to an entity without the final name.
         * @code
         * namespace Foo {
         *     class Bar{
         *         struct Hello{};
         *     };
         * }
         * @endcode
         * In such a case the parent mark for Hello is 'Foo::Bar'
         */
        static constexpr std::string_view parentsMark = "@@PARENTS_";

        /**
         * Function prefix - compiler adjuster strings. I.g.: inline keyword
         */
        static constexpr std::string_view funcPrefMark = "@@FUNC_PREF_";

        /**
         * Main namespace/struct name from the config. By default: 'R'
         */
        static constexpr std::string_view namespaceMark = "@@NAMESPACE_";
        static constexpr std::string_view onlyNameMark = "@@ONLY_NAME_";

        struct TokenEntry final
        {
            struct Hasher
            {
                std::size_t operator()(const TokenEntry& entry) const noexcept
                {
                    return std::hash<std::size_t>{}(entry.begin);
                }
            };

            static constexpr std::size_t invalidPosition = std::numeric_limits<std::size_t>::max();
            std::size_t begin = invalidPosition;

            void requireValidTokenBasedOnContent(const std::string& content) const;
            [[nodiscard]] constexpr bool isValid() const noexcept
            {
                return begin != invalidPosition;
            }
            [[nodiscard]] constexpr bool operator==(const TokenEntry& other) const noexcept
            {
                return begin == other.begin;
            }
            [[nodiscard]] constexpr bool operator<(const TokenEntry& other) const noexcept
            {
                return begin < other.begin;
            }
            [[nodiscard]] constexpr bool operator>(const TokenEntry& other) const noexcept
            {
                return begin > other.begin;
            }
        };

        struct BaseTokenData
        {
            virtual ~BaseTokenData() = default;

            std::string name;
            std::string parentSpace;

            [[nodiscard]] virtual std::string fullNamePath() const;
        };

        void warnMessage(const char* source, std::size_t indexInFileWithError,
                         const std::string& filepath, const std::string& errorMessage);

        void errorMessage(const char* source, std::size_t indexInFileWithError,
                          const std::string& filepath, const std::string& errorMessage);

        [[nodiscard]] static std::string PrettyPrintScope(const Scope* scope);
        [[nodiscard]] static std::string PrettyPrintIdentifier(const Scope* scope);

        [[nodiscard]] virtual std::string onGenerateHeaderFile(FileData& data) const = 0;
        [[nodiscard]] virtual std::string onGenerateSourceFile(FileData& data) const { return {}; }

        virtual void onScan(const FileData& content) = 0;

        [[nodiscard]] static std::size_t findTriggerKeyword(const std::string& content,
                                                            std::string_view keyword,
                                                            std::size_t offset);

    private:
        void PutMessage(Severity severity, const char* source, std::size_t indexInFileWithError,
                        const std::string& filepath, const std::string& errorMessage);

    protected:
        std::vector<TokenEntry> _tokens;
        bool _isSupportImplTranslationUnit = false;
        bool _hasImplTranslationUnit = false;
        const Config* _config = nullptr;
        FileProcessor* _parentFileProcessor = nullptr;

    private:
        std::unordered_map<Severity, int> _severityTraces;
    };

    template<class T>
    concept IsBaseReflector = std::derived_from<std::remove_reference_t<T>, BaseReflector>;

} // namespace JRM
