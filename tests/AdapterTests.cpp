#include "JustReflectMe/Adapter.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(AdapterTests, ReadsMissingFieldsWithDefaultAndCapturesStatus)
{
    RResourceStream<RJsonResourceStream> stream;
    int value = 0;

    stream.read("missing", value, 42);

    EXPECT_EQ(value, 42);
    ASSERT_THAT(stream.logs(), testing::ElementsAre(testing::Pair("missing", RStatus::NotFound)));
}

TEST(AdapterTests, ThrowsForRequiredMissingField)
{
    RResourceStream<RJsonResourceStream> stream;
    int value = 0;

    EXPECT_THROW(stream.read("required", value, 0, RPoint::Required), std::runtime_error);
    ASSERT_THAT(stream.logs(), testing::ElementsAre(testing::Pair("required", RStatus::NotFound)));
}

TEST(AdapterTests, WritesAndReadsJsonFieldsAndBuildsMetadataMap)
{
    RResourceStream<RJsonResourceStream> stream;
    stream.write("number", 7);

    int value = 0;
    stream.read("number", value, 0);
    EXPECT_EQ(value, 7);
    EXPECT_TRUE(stream.logs().empty());

    const std::vector<RClassField> fields = {
        RClassField{ .type = "int", .name = "id", .attribs = { "primary" } },
        RClassField{ .type = "std::string", .name = "name", .attribs = {} },
        RClassField{ .type = "long", .name = "id", .attribs = { "replacement" } },
    };
    const auto fieldsByName = RInternal::GetClassFieldsAsMap(fields);

    ASSERT_EQ(fieldsByName.size(), 2);
    EXPECT_EQ(fieldsByName.at("id").type, "long");
    EXPECT_THAT(fieldsByName.at("id").attribs, testing::ElementsAre("replacement"));
    EXPECT_EQ(RStatusToString(RStatus::Ok), "Ok");
    EXPECT_EQ(RStatusToString(RStatus::NotFound), "NotFound");
}
