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
