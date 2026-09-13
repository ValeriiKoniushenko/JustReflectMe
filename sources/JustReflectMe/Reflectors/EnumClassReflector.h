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

#include <unordered_map>

namespace JRM
{

    /**
     * @brief Reflects `ENUM_CLASS()`-marked scoped enumerations into `R<T>` specializations.
     *
     * Generated enum metadata provides conversion and enumeration helpers for the reflected
     * constants.
     */
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

        /**
         * @brief Returns the marker recognized by this reflector.
         * @return The `ENUM_CLASS` trigger keyword.
         */
        [[nodiscard]] constexpr std::string_view getTriggerKeyword() const noexcept override
        {
            return "ENUM_CLASS";
        }
        [[nodiscard]] std::optional<TypeMeta> findKnownTypeMeta(
            const std::string& fullPath) const override;
        void postScanCrossLinksResolving() override;

    protected:
        struct TokenData : public BaseTokenData
        {
            std::vector<std::pair<std::string, std::string>> constants;
        };

        static constexpr std::string_view countMark = "@@COUNT_";

        [[nodiscard]] std::string onGenerateHeaderFile(FileData& fileData) const override;
        void onScan(const FileData& fileData) override;

    private:
        [[nodiscard]] std::string generateSources(const TokenData& data) const;

    protected:
        std::unordered_map<TokenEntry, TokenData, TokenEntry::Hasher> _data;
    };

} // namespace JRM
