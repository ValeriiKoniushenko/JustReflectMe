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

#include "BaseReflector.h"
#include "JustReflectMe/FileNavigationHelper.h"

#include <unordered_map>

namespace JRM
{

    class ClassReflector : public BaseReflector
    {
    public:
        ClassReflector() = default;
        ClassReflector(const ClassReflector&) = default;
        ClassReflector& operator=(const ClassReflector&) = default;
        ClassReflector(ClassReflector&&) noexcept = default;
        ClassReflector& operator=(ClassReflector&&) noexcept = default;
        ~ClassReflector() override = default;

        [[nodiscard]] constexpr std::string_view getTriggerKeyword() const noexcept override
        {
            return "CLASS";
        }
        [[nodiscard]] std::set<std::string> getIncludes() const override;

    protected:
        struct FieldData
        {
            FileNavigator::Typename type;
            std::string name;
            std::string defaultValue;
            int flags = 0;
        };

        struct TokenData : public BaseTokenData
        {
            std::vector<FieldData> fields;
            std::vector<std::string> parents;
            std::vector<std::string> serializableParents;
            std::vector<std::string> attribs;
        };

        static constexpr std::string_view fieldNumbers = "@@FIELD_NUMBERS_";

        [[nodiscard]] std::string onGenerateHeaderFile(FileData& fileData) const override;
        void onScan(const FileData& fileData) override;

        void processFields(const Scope* classScope, const FileData& fileData, TokenData& data);

    private:
        [[nodiscard]] std::string generateSources(const TokenData& data) const;

    protected:
        std::unordered_map<TokenEntry, TokenData, TokenEntry::Hasher> _data;
    };

} // namespace JRM
