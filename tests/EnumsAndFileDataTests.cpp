// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "JustReflectMe/Enums.h"
#include "JustReflectMe/FileData.h"

#include "gtest/gtest.h"
#include <string>
#include <utility>

TEST(EnumsTests, ConvertsEveryContextTypeToText)
{
    EXPECT_EQ(JRM::ToString(JRM::ContextType::Undefined), "undefined");
    EXPECT_EQ(JRM::ToString(JRM::ContextType::File), "file");
    EXPECT_EQ(JRM::ToString(JRM::ContextType::Namespace), "namespace");
    EXPECT_EQ(JRM::ToString(JRM::ContextType::EnumClass), "enum class");
    EXPECT_EQ(JRM::ToString(JRM::ContextType::Class), "class");
    EXPECT_EQ(JRM::ToString(JRM::ContextType::Struct), "struct");
    // NOLINTNEXTLINE(bugprone-invalid-enum-cast) - exercise the defensive fallback.
    EXPECT_EQ(JRM::ToString(static_cast<JRM::ContextType>(99)), "undefined");
}

TEST(FileDataTests, StoresContentPathAndScopes)
{
    JRM::FileData data;
    JRM::PostProcessedFile content;
    content.content = "namespace NS { class Value {}; }";
    content.stringTokens.emplace(3, "original string");
    content.charTokens.emplace(4, "original char");

    data.setContent(std::move(content));
    data.setPath("include/value.h");

    EXPECT_EQ(data.getContent(), "namespace NS { class Value {}; }");
    EXPECT_EQ(data.getPath(), "include/value.h");
    EXPECT_EQ(data.getRealStringFromPlaceholderPos(3), "original string");
    EXPECT_EQ(data.getRealStringFromPlaceholderPos(99), "");
    EXPECT_EQ(data.getRealCharFromPlaceholderPos(4), "original char");
    EXPECT_EQ(data.getRealCharFromPlaceholderPos(99), "");

    data.scanScopes();
    const auto& scopes = data.getScopes();
    EXPECT_NE(scopes.getScopeAt(data.getContent().c_str() + data.getContent().find("Value")),
              nullptr);
    data.scanScopes();

    JRM::Scopes replacement;
    replacement.scan(data.getContent());
    data.setScope(std::move(replacement));
    EXPECT_NE(
        data.getScopes().getScopeAt(data.getContent().c_str() + data.getContent().find("Value")),
        nullptr);
}

TEST(FileDataTests, ResolvesStringLiteralByItsPlaceholderRunRegardlessOfOffsetDrift)
{
    // Preprocessing `a"x"b""c"yz"` masks every literal with a run of placeholders. Token keys are
    // positions in the unmasked text, so they do not line up with the masked positions.
    constexpr char p = JRM::PostProcessedFile::stringPlaceholder;
    JRM::PostProcessedFile content;
    content.content = std::string("a") + p + p + "b" + p + p + "c" + p + p + p + p;
    content.stringTokens.emplace(1, "x");
    content.stringTokens.emplace(5, "");
    content.stringTokens.emplace(8, "yz");

    JRM::FileData data;
    data.setContent(std::move(content));

    EXPECT_EQ(data.getStringLiteralAt(1), "x");
    EXPECT_EQ(data.getStringLiteralAt(2), "x");
    EXPECT_EQ(data.getStringLiteralAt(4), "");
    EXPECT_EQ(data.getStringLiteralAt(5), "");
    EXPECT_EQ(data.getStringLiteralAt(7), "yz");
    EXPECT_EQ(data.getStringLiteralAt(10), "yz");
    EXPECT_EQ(data.getStringLiteralAt(0), std::nullopt);
    EXPECT_EQ(data.getStringLiteralAt(3), std::nullopt);
    EXPECT_EQ(data.getStringLiteralAt(99), std::nullopt);
}
