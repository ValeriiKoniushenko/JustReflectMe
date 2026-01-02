

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

#include "Reflectors/BaseReflector.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace JRM
{

    bool Scope::isValid() const noexcept
    {
        return start != invalidPosition && end != invalidPosition;
    }

    bool Scope::operator==(const Scope& other) const noexcept
    {
        return start == other.start && end == other.end;
    }

    bool Scope::placedAtScope(std::size_t i) const noexcept
    {
        return i > start && i < end;
    }

    void Scopes::scan(const std::string& content)
    {
        if (content.empty()) [[unlikely]]
        {
            throw std::runtime_error("Can't scan scopes. The content is empty.");
        }

        int counter = 0;

        for (const char* p = content.c_str(); p && *p; ++p)
        {
            if (*p == '{')
            {
                _scopes.push_back({ .start = static_cast<std::size_t>(p - content.c_str()),
                                    .end = Scope::invalidPosition,
                                    .type = tryToDetermineScopeType(p, content.c_str()) });
                ++counter;
            }
            else if (*p == '}')
            {
                --counter;
                if (counter < 0) [[unlikely]]
                {
                    throw std::runtime_error("Found '}' without corresponding '{'.");
                }

                _scopes.at(counter).end = static_cast<std::size_t>(p - content.c_str());
            }
        }

        std::ranges::sort(_scopes,
                          [](const Scope& a, const Scope& b)
                          {
                              if (a.start != b.start)
                              {
                                  return a.start < b.start;
                              }
                              return a.end > b.end;
                          });
    }

    const Scope* Scopes::getTopScopeAtCursor(std::size_t cursor) const
    {
        auto it
            = std::upper_bound(_scopes.begin(), _scopes.end(), cursor,
                               [](std::size_t value, const Scope& s) { return value < s.start; });

        while (it != _scopes.begin())
        {
            --it;
            if (it->end > cursor)
            {
                return &(*it);
            }
        }
        return nullptr;
    }

    Scope::Type Scopes::tryToDetermineScopeType(const char* p, const char* start)
    {
        if (*p != '{')
        {
            return Scope::Type::Undefined;
        }

        --p;

        while (p > start && isspace(*p))
        {
            --p;
        }

        std::string buff;
        buff.reserve(64);
        while (p > start && *p != '\n')
        {
            buff.push_back(*p);
            --p;
        }
        std::ranges::reverse(buff);
        if (const auto pos = buff.find_first_not_of(" \t\n\r\f\v");
            pos != std::string::npos && pos != 0)
        {
            buff.erase(0, pos);
        }

        if (strncmp(buff.c_str(), "namespace", 9) == 0)
        {
            return Scope::Type::Namespace;
        }
        if (strncmp(buff.c_str(), "enum class", 10) == 0)
        {
            return Scope::Type::EnumClass;
        }

        return Scope::Type::Undefined;
    }

} // namespace JRM