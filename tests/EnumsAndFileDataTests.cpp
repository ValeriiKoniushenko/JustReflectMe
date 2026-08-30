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
