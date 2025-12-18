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

#pragma once
#include <concepts>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace JRM
{

    class SyntaxException : public std::runtime_error
    {
    public:
        SyntaxException(const std::string& message, std::size_t indexInFile);

        [[nodiscard]] std::string getFullMessage(const std::string& content, const std::string& pathToFile) const;

    protected:
        std::size_t _indexInFile = std::numeric_limits<std::size_t>::max();
    };

    class BaseReflector
    {
    public:
        static constexpr const char* warningCommentAtFileTop = R"(/*
 * This code was generated automatically with
 * https://github.com/ValeriiKoniushenko/JustReflectMe
 *
 * DO NOT EDIT MANUALLY!
 * Your changes will be replaced next time
 */

)";

        static constexpr const char* namespaceName = "Reflect";

        BaseReflector() = default;
        BaseReflector(const BaseReflector&) = default;
        BaseReflector& operator=(const BaseReflector&) = default;
        BaseReflector(BaseReflector&&) noexcept = default;
        BaseReflector& operator=(BaseReflector&&) noexcept = default;
        virtual ~BaseReflector() = default;

        [[nodiscard]] bool canProcessContent(const std::string& content) const;
        void scanContent(const std::string& content);

        [[nodiscard]] virtual const char* getTriggerKeyword() const noexcept = 0;
        [[nodiscard]] std::string generateHeaderFile(const std::string& newHeaderPath) const;
        [[nodiscard]] std::string generateSourceFile(const std::string& newHeaderPath) const;

    protected:
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

            [[nodiscard]] constexpr bool isValid() const noexcept
            {
                return begin != invalidPosition;
            }
            [[nodiscard]] constexpr bool operator==(const TokenEntry& other) const noexcept
            {
                return begin == other.begin;
            }
        };

        [[nodiscard]] virtual std::string onGenerateHeaderFile() const = 0;
        [[nodiscard]] virtual std::string onGenerateSourceFile() const = 0;
        virtual void onScan(const std::string& content) = 0;

    protected:
        std::vector<TokenEntry> _tokens;
    };

    template<class T>
    concept IsBaseReflector = std::derived_from<std::remove_reference_t<T>, BaseReflector>;

} // namespace JRM
