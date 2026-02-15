

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

#include "BaseReflector.h"

#include <unordered_map>

namespace JRM
{

    class EnumClassReflector : public BaseReflector
    {
    public:
        EnumClassReflector() = default;
        EnumClassReflector(const EnumClassReflector&) = default;
        EnumClassReflector& operator=(const EnumClassReflector&) = default;
        EnumClassReflector(EnumClassReflector&&) noexcept = default;
        EnumClassReflector& operator=(EnumClassReflector&&) noexcept = default;
        ~EnumClassReflector() override = default;

        [[nodiscard]] std::set<std::string> getIncludes() const override;

        [[nodiscard]] const char* getTriggerKeyword() const noexcept override
        {
            return "ENUM_CLASS";
        }

    protected:
        struct TokenData
        {
            std::string name;
            std::vector<std::pair<std::string, std::string>> constants;
            std::string parentSpace;

            [[nodiscard]] std::string fullNamePath() const;
        };

        static constexpr std::string_view countMark = "@@COUNT_";
        static constexpr std::string_view onlyNameMark = "@@ONLY_NAME_";

        [[nodiscard]] std::string onGenerateHeaderFilePreNamespace(
            FileData& data, const Config& config) const override;
        [[nodiscard]] std::string onGenerateHeaderFile(FileData& fileData,
                                                       const Config& config) const override;
        [[nodiscard]] std::string onGenerateSourceFile(FileData& fileData,
                                                       const Config& config) const override;
        void onScan(const FileData& fileData) override;

    private:
        [[nodiscard]] std::string generateSources(const TokenData& data,
                                                  const Config& config) const;

    protected:
        std::unordered_map<TokenEntry, TokenData, TokenEntry::Hasher> _data;
    };

} // namespace JRM
