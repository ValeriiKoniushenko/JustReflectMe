// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

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
    /**
     * @brief Describes one lexical scope in a source file.
     *
     * Scope boundaries point into the string passed to `Scopes::scan`. Child scopes form a tree,
     * and `type` identifies recognized namespaces, classes, structs, and enum classes.
     */
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

        /**
         * @brief Builds a scope tree for preprocessed C++ source text.
         * @param content Source text whose braces and declarations should be scanned.
         * @throw std::runtime_error If the content is empty or has malformed/unbalanced scopes.
         */
        void scan(const std::string& content);

        /**
         * @brief Finds the deepest scope containing a character position.
         * @param p Position inside the text previously passed to `scan`.
         * @return The deepest containing scope, or `nullptr` when no scope contains `p`.
         */
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
