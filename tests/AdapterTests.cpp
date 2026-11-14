// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

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
        RClassField{
            .type = "int",
            .name = "id",
            .attributes = RClassField::Attributes{ .attribs{ "primary" },
                                                   .min = std::nullopt,
                                                   .max = std::nullopt,
                                                   .step = std::nullopt,
                                                 },
        },
        RClassField{
            .type = "std::string",
            .name = "name",
            .attributes = RClassField::Attributes{},
        },
        RClassField{
            .type = "long",
            .name = "id",
            .attributes = RClassField::Attributes{ .attribs{ "replacement" },
            .min = std::nullopt,
            .max = std::nullopt, 
            .step = std::nullopt,
            },
        },
    };
    const auto fieldsByName = RInternal::GetClassFieldsAsMap(fields);

    ASSERT_EQ(fieldsByName.size(), 2);
    EXPECT_EQ(fieldsByName.at("id").type, "long");
    EXPECT_THAT(fieldsByName.at("id").attributes.attribs, testing::ElementsAre("replacement"));
    EXPECT_EQ(RStatusToString(RStatus::Ok), "Ok");
    EXPECT_EQ(RStatusToString(RStatus::NotFound), "NotFound");
}

TEST(AdapterTests, AttributesAnswerFlagAndMetaQueries)
{
    const RClassField field{
        .type = "float",
        .name = "speed",
        .attributes = RClassField::Attributes{
            .flags = RPoint::Required | RPoint::Meta,
            .meta = { RMetaEntry{ "Hidden", "" }, RMetaEntry{ "DisplayName", "Speed" } },
        },
    };

    EXPECT_TRUE(field.attributes.hasFlag(RPoint::Required));
    EXPECT_FALSE(field.attributes.hasFlag(RPoint::SuppressLimits));
    EXPECT_TRUE(field.attributes.hasMeta("Hidden"));
    EXPECT_EQ(field.attributes.getMeta("Hidden"), std::optional<std::string_view>(""));
    EXPECT_EQ(field.attributes.getMeta("DisplayName"), std::optional<std::string_view>("Speed"));
    EXPECT_EQ(field.attributes.getMeta("Missing"), std::nullopt);

    const auto byName = RInternal::GetClassFieldsAsMap({ field });
    EXPECT_THAT(byName.at("speed").attributes.meta,
                testing::ElementsAre(RMetaEntry{ "Hidden", "" },
                                     RMetaEntry{ "DisplayName", "Speed" }));
}
