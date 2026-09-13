// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "BaseReflector.h"
#include "JustReflectMe/FileNavigationHelper.h"

#include <unordered_map>

namespace JRM
{

    /**
     * @brief Reflects `CLASS()`-marked classes and structs into `R<T>` specializations.
     *
     * Class reflection includes marked fields and supports generated serialization helpers and
     * field metadata accessors.
     */
    class ClassReflector : public BaseReflector
    {
    public:
        ClassReflector() = default;
        ClassReflector(const ClassReflector&) = default;
        ClassReflector& operator=(const ClassReflector&) = default;
        ClassReflector(ClassReflector&&) noexcept = default;
        ClassReflector& operator=(ClassReflector&&) noexcept = default;
        ~ClassReflector() override = default;

        /**
         * @brief Returns the marker recognized by this reflector.
         * @return The `CLASS` trigger keyword.
         */
        [[nodiscard]] constexpr std::string_view getTriggerKeyword() const noexcept override
        {
            return "CLASS";
        }
        [[nodiscard]] std::optional<TypeMeta> findKnownTypeMeta(
            const std::string& fullPath) const override;
        void postScanCrossLinksResolving() override;

        [[nodiscard]] std::set<std::string> getIncludes() const override;

    protected:
        struct FieldData
        {
            std::vector<std::string> attributes;
            FileNavigator::Typename type;
            std::string name;
            std::string defaultValue;
            int flags = 0;
            std::optional<TypeMeta> typeMeta;
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
