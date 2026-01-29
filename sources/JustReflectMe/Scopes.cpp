

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

#include "Scopes.h"

#include "FileNavigationHelper.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace JRM
{

    bool Scope::isValid() const noexcept
    {
        return start && end;
    }

    bool Scope::operator==(const Scope& other) const noexcept
    {
        return start == other.start && end == other.end;
    }

    bool Scope::contains(const char* i) const noexcept
    {
        return i > start && i < end;
    }

    const Scope* Scope::findDeepest(const char* i) const
    {
        if (contains(i))
        {
            const Scope* found = this;
            for (const auto& child : children)
            {
                if (const auto* result = child.findDeepest(i))
                {
                    found = result;
                    break;
                }
            }

            return found;
        }

        return nullptr;
    }

    std::string Scope::getIdentifier() const
    {
        return FileNavigator::ReadAsIdentifier(identifierStart);
    }

    void Scope::revalidateTree()
    {
        for (auto& child : children)
        {
            child.parent = this;
            child.revalidateTree();
        }
    }

    void Scopes::scan(const std::string& content)
    {
        if (content.empty()) [[unlikely]]
        {
            throw std::runtime_error("Can't scan scopes. The content is empty.");
        }

        _root.type = Scope::Type::File;
        _root.start = content.c_str();
        _root.end = content.c_str() + content.size() - 1;

        Scope* parent = &_root;

        for (const char* p = content.c_str(); p && *p; ++p)
        {
            if (*p == '{')
            {
                auto* child = &parent->children.emplace_back();
                child->parent = parent;
                parent = child;
                parent->start = p;
                parent->end = nullptr;
                tryToDetermineScopeType(*parent, p, content.c_str());
            }
            else if (*p == '}')
            {
                if (!parent) [[unlikely]]
                {
                    throw std::runtime_error(
                        "Can't scan scopes. The content contains unmatched '{'.");
                }

                parent->end = p;
                parent = parent->parent;
            }
        }

        // Updating/revalidating all parents
        _root.revalidateTree();
    }

    const Scope* Scopes::getScopeAt(const char* p) const
    {
        if (_root.isValid())
        {
            return _root.findDeepest(p);
        }

        return nullptr;
    }

    void Scopes::tryToDetermineScopeType(Scope& scope, const char* p, const char* start)
    {
        if (*p != '{')
        {
            return;
        }

        --p;

        while (p > start && isspace(*p))
        {
            --p;
        }

        while (p > start && *p != '\n')
        {
            --p;
        }
        while (*p && isspace(*p))
        {
            ++p;
        }

        static const std::string namespaceKeyword = "namespace";
        static const std::string enumClassKeyword = "enum class";
        static const std::string classKeyword = "class";

        if (strncmp(p, namespaceKeyword.c_str(), namespaceKeyword.size()) == 0)
        {
            scope.type = Scope::Type::Namespace;
            scope.identifierStart = FileNavigator::GoToNotSpace(p + namespaceKeyword.size());
        }
        else if (strncmp(p, enumClassKeyword.c_str(), enumClassKeyword.size()) == 0)
        {
            scope.type = Scope::Type::EnumClass;
            scope.identifierStart = FileNavigator::GoToNotSpace(p + enumClassKeyword.size());
        }
        else if (strncmp(p, classKeyword.c_str(), classKeyword.size()) == 0)
        {
            scope.type = Scope::Type::Class;
            scope.identifierStart = FileNavigator::GoToNotSpace(p + classKeyword.size());
        }
        else
        {
            scope.identifierStart = nullptr;
            scope.type = Scope::Type::Undefined;
        }
    }

} // namespace JRM