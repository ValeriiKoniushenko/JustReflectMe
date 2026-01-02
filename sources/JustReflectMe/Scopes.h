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
#include <limits>
#include <string>
#include <vector>

namespace JRM
{
    struct Scope
    {
        enum class Type
        {
            Undefined,
            Namespace,
            EnumClass
        };

        static constexpr std::size_t invalidPosition = std::numeric_limits<std::size_t>::max();

        [[nodiscard]] bool isValid() const noexcept;
        [[nodiscard]] bool operator==(const Scope& other) const noexcept;
        [[nodiscard]] bool placedAtScope(std::size_t i) const noexcept;

        std::size_t start = invalidPosition;
        std::size_t end = invalidPosition;
        Type type = Type::Undefined;
    };

    class Scopes
    {
    public:

        Scopes() = default;
        Scopes(const Scopes&) = default;
        Scopes& operator=(const Scopes&) = default;
        Scopes(Scopes&&) noexcept = default;
        Scopes& operator=(Scopes&&) noexcept = default;
        virtual ~Scopes() = default;

        void scan(const std::string& content);

        [[nodiscard]] const Scope* getTopScopeAtCursor(std::size_t cursor) const;

    private:
        [[nodiscard]] Scope::Type tryToDetermineScopeType(const char* p, const char* start);

    protected:
        std::vector<Scope> _scopes;
    };

} // namespace JRM
