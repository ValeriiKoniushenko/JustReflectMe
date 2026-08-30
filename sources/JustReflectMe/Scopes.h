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

#include "Enums.h"

#include <limits>
#include <memory>
#include <string>
#include <vector>

#if defined(JRM_ENABLE_TESTS)
    #include "gtest/gtest_prod.h"
#endif

namespace JRM
{
    struct Scope
    {
        enum Attr
        {
            Attr_None = 0,
            Attr_Template = 1 << 0
        };

        [[nodiscard]] bool isValid() const noexcept;
        [[nodiscard]] bool operator==(const Scope& other) const noexcept;
        [[nodiscard]] bool contains(const char* i) const noexcept;
        [[nodiscard]] const Scope* findDeepest(const char* i) const;
        [[nodiscard]] std::string getIdentifier() const;
        void revalidateTree();

        std::vector<Scope> children;
        const char* identifierStart = nullptr;
        const char* start = nullptr;
        const char* end = nullptr;
        Scope* parent = nullptr;
        ContextType type = ContextType::Undefined;
        int attribute = Attr_None;
    };

    class Scopes
    {
    public:
        Scopes() = default;
        Scopes(const Scopes&) = delete;
        Scopes& operator=(const Scopes&) = delete;
        Scopes(Scopes&&) noexcept = default;
        Scopes& operator=(Scopes&&) noexcept = default;
        virtual ~Scopes() = default;

        void scan(const std::string& content);

        [[nodiscard]] const Scope* getScopeAt(const char* p) const;

    private:
        static void tryToDetermineScopeType(Scope& scope, const char* p, const char* start);
        static void tryToDetermineScopeAttribute(Scope& scope, const char* p, const char* start);

#if defined(JRM_ENABLE_TESTS)
        FRIEND_TEST(ScopesTests, IgnoresNonOpeningScopeStarts);
#endif

    protected:
        Scope _root;
    };

} // namespace JRM
