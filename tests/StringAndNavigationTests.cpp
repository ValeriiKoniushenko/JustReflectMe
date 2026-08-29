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

TEST(StringHelperTests, TrimsAndSplitsWithOptionalWhitespaceRemoval)
{
    std::string padded = " \t value\n";
    StringHelper::TrimInPlace(padded);
    EXPECT_EQ(padded, "value");

    EXPECT_THAT(StringHelper::SplitString(" first, second ,, fourth "),
                testing::ElementsAre("first", "second", "", "fourth"));
    EXPECT_THAT(StringHelper::SplitString(" first, second ", ',', false),
                testing::ElementsAre(" first", " second "));
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
    EXPECT_EQ(FileNavigator::FindOnThisLine(final, "keyword"), final + std::string_view("final ").size());
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
