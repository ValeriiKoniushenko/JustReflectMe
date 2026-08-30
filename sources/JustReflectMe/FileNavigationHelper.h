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

#include "FileProcessor.h"

#include <string>
#include <vector>

/**
 * @brief Lightweight pointer-based navigation and token-reading helpers for preprocessed C++ text.
 *
 * Every pointer-returning function returns a position in the input buffer; it never allocates or
 * extends that buffer. Unless a function explicitly accepts `nullptr`, its input must point into a
 * NUL-terminated character sequence. `IsNewLine` and the line-oriented helpers recognize both a
 * physical newline and `JRM::PostProcessedFile::newLinePlaceholder`, which represents a newline
 * after file preprocessing.
 *
 * These helpers deliberately provide only the lexical operations needed by the reflectors. They
 * do not parse comments, string literals, or the complete C++ grammar.
 *
 * @code
 * const std::string content = "first\nsecond keyword";
 * const char* secondLine = FileNavigator::GoToNextLine(content.c_str());
 * const char* keyword = FileNavigator::FindWordOnThisLine(secondLine, "keyword");
 * // `keyword` points into `content`, immediately after "second ".
 * @endcode
 *
 * @see benchmarks/StringAndNavigationBenchmarks.cpp (`BM_GoToLineStart`,
 * `BM_FindFirstWithLineLimit`, and `BM_GetLineNumberAndColumn`).
 */
namespace FileNavigator
{

    /**
     * @brief Finds the first character of the line that contains a position.
     * @param p Position inside the source buffer.
     * @param begin First character of the source buffer.
     * @return The first character after the preceding recognized newline, or `nullptr` when
     * either pointer is null.
     * @pre `p` belongs to the NUL-terminated buffer beginning at `begin`.
     * @see benchmarks/StringAndNavigationBenchmarks.cpp (`BM_GoToLineStart`).
     */
    [[nodiscard]] const char* GoToLineStart(const char* p, const char* begin);

    /**
     * @brief Finds the first character of the line preceding a position.
     * @param p Position inside the source buffer.
     * @param begin First character of the source buffer.
     * @return The previous line's first character; returns `begin` when `p` is on the first line,
     * or `nullptr` when either pointer is null.
     * @pre `p` belongs to the NUL-terminated buffer beginning at `begin`.
     */
    [[nodiscard]] const char* GoToPrevLine(const char* p, const char* begin);

    /**
     * @brief Finds the first character of the next line.
     * @param p Start position from which to search for a recognized newline.
     * @return The character after the next `\n` or newline placeholder, or `nullptr` when none is
     * found.
     * @pre `p` is non-null and points to a NUL-terminated character sequence.
     */
    [[nodiscard]] const char* GoToNextLine(const char* p);

    /**
     * @brief Finds the first substring occurrence when it starts within a line limit.
     *
     * A limit of zero restricts the search to `source`'s current line; a limit of one also permits
     * the next line. The function checks only the first occurrence of `keyword` in the remaining
     * buffer, so it does not continue searching after that occurrence exceeds the limit.
     *
     * @param source Start of the search range.
     * @param keyword NUL-terminated substring to find.
     * @param limit Maximum number of recognized newline boundaries before the match.
     * @return A pointer to the first occurrence when it is within the limit, otherwise `nullptr`.
     * @see benchmarks/StringAndNavigationBenchmarks.cpp (`BM_FindFirstWithLineLimit`).
     */
    [[nodiscard]] const char* FindFirstWithLineLimit(const char* source, const char* keyword,
                                                     std::size_t limit);

    /**
     * @brief Finds the first substring occurrence that begins before the next recognized newline.
     * @param source Start of the line to search.
     * @param keyword NUL-terminated substring to find.
     * @return A pointer to the first substring occurrence on the current line, or `nullptr`.
     * @pre `source` and `keyword` are non-null NUL-terminated character sequences.
     * @note This is a substring search; it does not require token boundaries.
     */
    [[nodiscard]] const char* FindOnThisLine(const char* source, const char* keyword);

    /**
     * @brief Finds the first whole-word occurrence that begins before the next recognized newline.
     * @param content NUL-terminated source text beginning at the line to search.
     * @param word NUL-terminated word to find.
     * @return A pointer to the first occurrence when it has valid word boundaries, or `nullptr`.
     * @pre `content.data()` and `word.data()` refer to NUL-terminated character sequences.
     * @note Only the first substring occurrence is considered. If it is embedded in a larger word,
     * the function returns `nullptr` without checking later occurrences on the line.
     */
    [[nodiscard]] const char* FindWordOnThisLine(std::string_view content, std::string_view word);

    /**
     * @brief Advances to the next ASCII space or horizontal tab.
     * @param source Start position.
     * @return The first space, tab, or terminating NUL; returns `nullptr` for null input.
     * @note Newlines are not treated as spaces by this function.
     */
    [[nodiscard]] const char* GoToSpace(const char* source);

    /**
     * @brief Advances to the next whitespace or recognized newline character.
     * @param source Start position.
     * @return The first character accepted by `IsSpace`, or the terminating NUL; returns `nullptr`
     * for null input.
     */
    [[nodiscard]] const char* GoToBlank(const char* source);

    /**
     * @brief Skips ASCII spaces and horizontal tabs.
     * @param source Start position.
     * @return The first non-space/non-tab character, the terminating NUL, or `nullptr` for null
     * input.
     * @note Newlines are not skipped.
     */
    [[nodiscard]] const char* GoToNotSpace(const char* source);

    /**
     * @brief Skips all characters accepted by `IsSpace`.
     * @param source Start position.
     * @return The first non-whitespace character or the terminating NUL.
     * @pre `source` is non-null and points to a NUL-terminated character sequence.
     */
    [[nodiscard]] const char* SkipAllBlanks(const char* source);

    /**
     * @brief Reads a simple C++-style identifier, including a qualified name.
     *
     * Leading ASCII spaces and horizontal tabs are ignored. The first character must be alphabetic
     * or `_`; subsequent characters may also be digits or `:`. The result is lexical only and does
     * not validate the placement or number of scope separators.
     *
     * @param source Start position.
     * @return The identifier text, or an empty string for null input or an invalid first character.
     */
    [[nodiscard]] std::string ReadAsIdentifier(const char* source);

    /**
     * @brief Converts a zero-based character offset to a one-based line number.
     * @param source NUL-terminated source text.
     * @param i Zero-based offset from `source`.
     * @return The one-based line number, or zero for null input.
     * @pre `i` does not exceed the length of `source`.
     */
    [[nodiscard]] std::size_t GetLineNumber(const char* source, std::size_t i);

    /**
     * @brief Converts a zero-based character offset to one-based line and column numbers.
     * @param source NUL-terminated source text.
     * @param i Zero-based offset from `source`.
     * @return `{ line, column }`, both one-based; returns `{ 0, 0 }` for null input.
     * @pre `i` does not exceed the length of `source`.
     * @see benchmarks/StringAndNavigationBenchmarks.cpp (`BM_GetLineNumberAndColumn`).
     */
    [[nodiscard]] std::pair<std::size_t, std::size_t> GetLineNumberAndColumn(const char* source,
                                                                             std::size_t i);

    /**
     * @brief Finds the matching close delimiter for a nested delimiter sequence.
     *
     * The first character must be one of `{`, `(`, `<`, or `[`. Nesting is tracked only for that
     * delimiter pair; this function does not parse other C++ syntax, comments, or string literals.
     *
     * @code
     * const char* end = FileNavigator::FindScopeEnd("{ outer { inner } }");
     * // `end` points to the final `}`.
     * @endcode
     *
     * @param source Start of the delimiter sequence.
     * @return The matching `}`, `)`, `>`, or `]`, or `nullptr` for invalid or unbalanced input.
     * @see benchmarks/StringAndNavigationBenchmarks.cpp (`BM_FindScopeEnd`).
     */
    [[nodiscard]] const char* FindScopeEnd(const char* source);

    /**
     * @brief Checks whether a known word occurrence has C++ identifier boundaries.
     * @param content Text containing the occurrence.
     * @param word Word assumed to start at `wordPos`.
     * @param wordPos Zero-based starting position of `word` in `content`.
     * @return `true` when neither adjacent character is alphanumeric or `_`.
     * @pre `wordPos` is valid and `content.substr(wordPos, word.size()) == word`.
     * @note The function validates boundaries only; it does not compare `word` with `content`.
     */
    [[nodiscard]] bool IsWord(std::string_view content, std::string_view word, std::size_t wordPos);

    /**
     * @brief Finds the first prefix that matches the beginning of a C string.
     * @param content NUL-terminated text to inspect.
     * @param prefixes Prefixes to try in order.
     * @return The zero-based index of the first matching prefix, or `-1` when none matches.
     * @pre `content` is non-null and has enough accessible characters for each checked prefix.
     * @note This does not validate word boundaries.
     */
    [[nodiscard]] int StartWith(const char* content, const std::vector<std::string_view>& prefixes);

    /**
     * @brief Tests whether a C string begins with a prefix.
     * @param content NUL-terminated text to inspect.
     * @param prefix Prefix to compare.
     * @return `true` when the first `prefix.size()` characters match.
     * @pre `content` is non-null and has enough accessible characters for `prefix`.
     * @note This does not validate word boundaries.
     */
    [[nodiscard]] bool StartWith(const char* content, std::string_view prefix);

    /**
     * @brief Tests whether whitespace before the next token reaches a physical newline.
     * @param content Start position to inspect.
     * @return `true` when ASCII whitespace includes `\n` before a non-whitespace character;
     * returns `false` for null input.
     * @note Unlike `IsNewLine`, this function does not recognize the preprocessing newline
     * placeholder.
     */
    [[nodiscard]] bool LeadToNewLine(const char* content) noexcept;

    /**
     * @brief Tests whether a character is a physical or preprocessed newline.
     * @param ch Character value to inspect.
     * @return `true` for `\n` or `JRM::PostProcessedFile::newLinePlaceholder`.
     */
    [[nodiscard]] inline bool IsNewLine(int ch) noexcept
    {
        return ch == '\n' || ch == JRM::PostProcessedFile::newLinePlaceholder;
    }

    /**
     * @brief Tests whether a character is standard whitespace or a preprocessed newline.
     * @param ch Character value to inspect.
     * @return `true` when `std::isspace(ch)` or `IsNewLine(ch)` is true.
     */
    [[nodiscard]] inline bool IsSpace(int ch) noexcept
    {
        return std::isspace(ch) || IsNewLine(ch);
    }

    /**
     * @brief Parsed type name together with selected declaration attributes.
     *
     * `name` stores the type spelling without a leading or trailing `const`; `isConst` preserves
     * that qualifier separately. `attribute` records one supported declaration prefix.
     */
    struct Typename
    {
        /**
         * @brief Declaration prefix recognized by `ReadAsTypename`.
         */
        enum class Attribute
        {
            None,
            Constexpr,
            Static,
            Inline
        };

        /**
         * @brief Sets the recognized declaration prefix.
         * @param str Attribute spelling: `constexpr`, `static`, or `inline`.
         * @note An unrecognized value leaves `attribute` unchanged.
         */
        void setAttributeFromStr(std::string_view str);

        /**
         * @brief Reconstructs the type spelling with a leading `const` qualifier when present.
         * @return `"const " + name` when `isConst` is true; otherwise `name`.
         */
        [[nodiscard]] std::string getNameWithCV() const;

        /** @brief Parsed type spelling, including any trailing `*` or `&`. */
        std::string name;

        /** @brief Recognized declaration prefix. */
        Attribute attribute = Attribute::None;

        /** @brief Whether the source contained a leading or trailing `const` qualifier. */
        bool isConst = false;
    };

    /**
     * @brief Parses the focused type syntax used by the reflection field scanner.
     *
     * The parser recognizes one of `constexpr`, `static`, or `inline`; an optional leading or
     * trailing `const`; a qualified identifier with nested `<...>` template arguments; and trailing
     * `*` and `&` suffixes. It is not a full C++ type parser.
     *
     * @code
     * int offset = 0;
     * const auto type = FileNavigator::ReadAsTypename(
     *     "static const std::vector<int>* field", offset);
     * // type.name == "std::vector<int>*"
     * // type.attribute == FileNavigator::Typename::Attribute::Static
     * // type.isConst == true
     * @endcode
     *
     * @param source Start of a type declaration; leading whitespace is not skipped.
     * @param offset Receives the parser's stopping offset relative to `source` on a successful
     * parse.
     * @return Parsed type metadata, or a default-constructed value when `source` is null or does
     * not begin with an alphabetic character or `_`.
     * @note `offset` is left unchanged when parsing returns the default value.
     * @see benchmarks/StringAndNavigationBenchmarks.cpp (`BM_ReadAsTypename`).
     */
    [[nodiscard]] Typename ReadAsTypename(const char* source, int& offset);

} // namespace FileNavigator
