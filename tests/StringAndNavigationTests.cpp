#include "JustReflectMe/FileNavigationHelper.h"
#include "JustReflectMe/StringHelper.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(StringHelperTests, ReplacesEveryOccurrenceAndHandlesReplacementGrowth)
{
    std::string value = "one-two-one";

    StringHelper::FindAndReplaceAll(value, "one", "three");

    EXPECT_EQ(value, "three-two-three");
}

TEST(StringHelperTests, RemovesMatchesAndLeavesStringsWithoutMatchesUntouched)
{
    std::string values = "red,green,red";
    StringHelper::FindAndReplaceAll(values, "red", "");
    EXPECT_EQ(values, ",green,");

    std::string unchanged = "blue";
    StringHelper::FindAndReplaceAll(unchanged, "red", "green");
    EXPECT_EQ(unchanged, "blue");
}

TEST(StringHelperTests, IgnoresEmptySearchPatterns)
{
    std::string value = "stable";

    StringHelper::FindAndReplaceAll(value, "", "replacement");

    EXPECT_EQ(value, "stable");
}

TEST(StringHelperTests, TrimsLeadingAndTrailingWhitespaceWithoutChangingInternalWhitespace)
{
    std::string padded = " \t value\r\n";
    StringHelper::TrimInPlace(padded);
    EXPECT_EQ(padded, "value");

    std::string internalWhitespace = "alpha  beta";
    StringHelper::TrimInPlace(internalWhitespace);
    EXPECT_EQ(internalWhitespace, "alpha  beta");

    std::string onlyWhitespace = " \t\v\f\r\n";
    StringHelper::TrimInPlace(onlyWhitespace);
    EXPECT_TRUE(onlyWhitespace.empty());

    std::string empty;
    StringHelper::TrimInPlace(empty);
    EXPECT_TRUE(empty.empty());
}

TEST(StringHelperTests, SplitsWithDefaultDelimiterAndOptionalWhitespaceRemoval)
{
    EXPECT_THAT(StringHelper::SplitString(" first, second ,, fourth "),
                testing::ElementsAre("first", "second", "", "fourth"));
    EXPECT_THAT(StringHelper::SplitString(" first, second ", ',', false),
                testing::ElementsAre(" first", " second "));
}

TEST(StringHelperTests, SplitsCustomDelimitersAndDefinesBoundaryBehavior)
{
    EXPECT_THAT(StringHelper::SplitString("one| two |three", '|'),
                testing::ElementsAre("one", "two", "three"));
    EXPECT_THAT(StringHelper::SplitString("single value"), testing::ElementsAre("single value"));
    EXPECT_THAT(StringHelper::SplitString("tail,"), testing::ElementsAre("tail"));
    EXPECT_THAT(StringHelper::SplitString("   "), testing::ElementsAre(""));
    EXPECT_TRUE(StringHelper::SplitString("").empty());
}

TEST(FileNavigationHelperTests, NavigatesLinesAndFindsKeywordsOnFinalLine)
{
    const std::string content = "first line\nsecond keyword\nfinal keyword";
    const auto* begin = content.c_str();
    const auto* second = begin + content.find("second");
    const auto* final = begin + content.find("final");

    EXPECT_EQ(FileNavigator::GoToLineStart(second + 4, begin), second);
    EXPECT_EQ(FileNavigator::GoToPrevLine(second + 4, begin), begin);
    EXPECT_EQ(FileNavigator::GoToNextLine(begin), second);
    EXPECT_EQ(FileNavigator::FindFirstWithLineLimit(begin, "keyword", 1),
              second + std::string_view("second ").size());
    EXPECT_EQ(FileNavigator::FindFirstWithLineLimit(begin, "keyword", 0), nullptr);
    EXPECT_EQ(FileNavigator::FindOnThisLine(final, "keyword"),
              final + std::string_view("final ").size());
    EXPECT_EQ(FileNavigator::FindWordOnThisLine("signal signalExtra", "signal"),
              std::string_view("signal signalExtra").data());
}

TEST(FileNavigationHelperTests, CalculatesLineAndColumnAndReadsTypenames)
{
    const std::string content = "one\ntwo\nthree";
    EXPECT_EQ(FileNavigator::GetLineNumber(content.c_str(), 8), 3);
    EXPECT_EQ(FileNavigator::GetLineNumberAndColumn(content.c_str(), 8),
              (std::pair<std::size_t, std::size_t>{ 3, 1 }));
    EXPECT_EQ(FileNavigator::GetLineNumberAndColumn(content.c_str(), 11),
              (std::pair<std::size_t, std::size_t>{ 3, 4 }));

    int offset = 0;
    const auto type = FileNavigator::ReadAsTypename("static const std::vector<int>* field", offset);
    EXPECT_EQ(type.name, "std::vector<int>*");
    EXPECT_TRUE(type.isConst);
    EXPECT_EQ(type.attribute, FileNavigator::Typename::Attribute::Static);
    EXPECT_GT(offset, 0);
}

TEST(FileNavigationHelperTests, DetectsWhitespaceLeadingToNewline)
{
    EXPECT_TRUE(FileNavigator::LeadToNewLine(" \t\nvalue"));
    EXPECT_FALSE(FileNavigator::LeadToNewLine(" \tvalue"));
    EXPECT_FALSE(FileNavigator::LeadToNewLine(nullptr));
}

TEST(FileNavigationHelperTests, HandlesNavigationSearchAndScopeBoundaries)
{
    const std::string lines = "first\r\nsecond\x1Dthird";
    const auto* begin = lines.c_str();
    const auto* second = begin + lines.find("second");
    const auto* third = begin + lines.find("third");

    EXPECT_EQ(FileNavigator::GoToLineStart(nullptr, begin), nullptr);
    EXPECT_EQ(FileNavigator::GoToLineStart(second + 2, begin), second);
    EXPECT_EQ(FileNavigator::GoToPrevLine(nullptr, begin), nullptr);
    EXPECT_EQ(FileNavigator::GoToPrevLine(begin, begin), begin);
    EXPECT_EQ(FileNavigator::GoToPrevLine(third + 1, begin), second);
    EXPECT_EQ(FileNavigator::GoToNextLine(begin), second);
    EXPECT_EQ(FileNavigator::GoToNextLine(second), third);
    EXPECT_EQ(FileNavigator::GoToNextLine(third), nullptr);

    EXPECT_EQ(FileNavigator::FindFirstWithLineLimit(nullptr, "first", 1), nullptr);
    EXPECT_EQ(FileNavigator::FindFirstWithLineLimit(begin, nullptr, 1), nullptr);
    EXPECT_EQ(FileNavigator::FindFirstWithLineLimit(begin, "missing", 1), nullptr);
    EXPECT_EQ(FileNavigator::FindFirstWithLineLimit(begin, "third", 1), nullptr);
    EXPECT_EQ(FileNavigator::FindOnThisLine(begin, "second"), nullptr);
    EXPECT_EQ(FileNavigator::FindWordOnThisLine("prefixWord word", "word"),
              std::string_view("prefixWord word").data() + 11);

    EXPECT_EQ(FileNavigator::GoToSpace("word\tvalue"), std::string_view("word\tvalue").data() + 4);
    EXPECT_EQ(FileNavigator::GoToBlank("word\nvalue"), std::string_view("word\nvalue").data() + 4);
    EXPECT_EQ(FileNavigator::GoToNotSpace(" \tvalue"), std::string_view(" \tvalue").data() + 2);
    EXPECT_EQ(FileNavigator::SkipAllBlanks(" \t\nvalue"),
              std::string_view(" \t\nvalue").data() + 3);
    EXPECT_EQ(FileNavigator::ReadAsIdentifier("  Namespace::Value rest"), "Namespace::Value");
    EXPECT_TRUE(FileNavigator::ReadAsIdentifier("  42").empty());
    EXPECT_TRUE(FileNavigator::ReadAsIdentifier(nullptr).empty());

    EXPECT_EQ(FileNavigator::GetLineNumber(nullptr, 2), 0);
    EXPECT_EQ(FileNavigator::GetLineNumberAndColumn(nullptr, 2),
              (std::pair<std::size_t, std::size_t>{ 0, 0 }));
    EXPECT_EQ(FileNavigator::FindScopeEnd(nullptr), nullptr);
    EXPECT_EQ(FileNavigator::FindScopeEnd("value"), nullptr);
    EXPECT_EQ(*FileNavigator::FindScopeEnd("{ { } }"), '}');
    EXPECT_EQ(*FileNavigator::FindScopeEnd("(())"), ')');
    EXPECT_EQ(*FileNavigator::FindScopeEnd("<><>"), '>');
    EXPECT_EQ(*FileNavigator::FindScopeEnd("[[]]"), ']');
    EXPECT_EQ(FileNavigator::FindScopeEnd("{ missing"), nullptr);

    EXPECT_FALSE(FileNavigator::IsWord("prefixWord", "Word", 6));
    EXPECT_FALSE(FileNavigator::IsWord("wordSuffix", "word", 0));
    EXPECT_TRUE(FileNavigator::IsWord(" word;", "word", 1));
    EXPECT_EQ(FileNavigator::StartWith("static value", { "constexpr", "static", "inline" }), 1);
    EXPECT_EQ(
        FileNavigator::StartWith("value", std::vector<std::string_view>{ "constexpr", "static" }),
        -1);
    EXPECT_TRUE(FileNavigator::StartWith("prefix", "pre"));
    EXPECT_FALSE(FileNavigator::StartWith("prefix", "post"));
}

TEST(FileNavigationHelperTests, ParsesTypenameVariantsAndAttributes)
{
    int offset = 0;
    EXPECT_TRUE(FileNavigator::ReadAsTypename(nullptr, offset).name.empty());
    EXPECT_TRUE(FileNavigator::ReadAsTypename("42", offset).name.empty());

    const auto constexprType = FileNavigator::ReadAsTypename("constexpr Widget&,", offset);
    EXPECT_EQ(constexprType.name, "Widget&");
    EXPECT_EQ(constexprType.attribute, FileNavigator::Typename::Attribute::Constexpr);
    EXPECT_FALSE(constexprType.isConst);

    const auto inlineType = FileNavigator::ReadAsTypename("inline const Value * field", offset);
    EXPECT_EQ(inlineType.name, "Value*");
    EXPECT_EQ(inlineType.attribute, FileNavigator::Typename::Attribute::Inline);
    EXPECT_TRUE(inlineType.isConst);
    EXPECT_EQ(inlineType.getNameWithCV(), "const Value*");

    const auto trailingConst = FileNavigator::ReadAsTypename("Value const", offset);
    EXPECT_EQ(trailingConst.name, "Value");
    EXPECT_TRUE(trailingConst.isConst);

    FileNavigator::Typename typenameValue;
    typenameValue.name = "Thing";
    typenameValue.setAttributeFromStr("static");
    EXPECT_EQ(typenameValue.attribute, FileNavigator::Typename::Attribute::Static);
    typenameValue.setAttributeFromStr("unknown");
    EXPECT_EQ(typenameValue.getNameWithCV(), "Thing");
}
