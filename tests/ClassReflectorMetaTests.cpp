// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "JustReflectMe/Adapter.h"
#include "JustReflectMe/Config.h"
#include "JustReflectMe/FileProcessor.h"
#include "JustReflectMe/Reflectors/ClassReflector.h"
#include "JustReflectMe/Reflectors/EnumClassReflector.h"
#include "TestSupport.h"

#include "gtest/gtest.h"

namespace
{
    struct GenerationResult
    {
        bool succeeded = false;
        int errors = 0;
        int warnings = 0;
        std::string generated;
    };

    GenerationResult Generate(const std::string& header)
    {
        TestSupport::TemporaryDirectory project;
        const auto path = project.writeFile("Model.h", header);

        JRM::Config config;
        JRM::FileProcessor processor;
        processor.registerReflector<JRM::EnumClassReflector>();
        processor.registerReflector<JRM::ClassReflector>();

        GenerationResult result;
        result.succeeded = processor.run(path, config);
        for (const auto& reflector : processor.getReflectors())
        {
            if (dynamic_cast<const JRM::ClassReflector*>(reflector.get()))
            {
                result.errors = reflector->numberOfErrors();
                result.warnings = reflector->numberOfWarnings();
            }
        }
        result.generated = project.readFile("Model.generated.h");
        return result;
    }

    std::string ClassWithField(const std::string& fieldMarker)
    {
        return "#pragma once\nCLASS();\nclass Model\n{\n    R_FRIEND(Model);\n    " + fieldMarker
               + "\n    int value = 0;\n};\n";
    }
} // namespace

TEST(ClassReflectorMetaTests, EmitsTypedVisitorsCachedFieldsAndBaseVisitor)
{
    const auto result = Generate(ClassWithField("FIELD();"));

    ASSERT_TRUE(result.succeeded);
    EXPECT_TRUE(result.generated.contains("GetFieldsArray()"));
    EXPECT_TRUE(result.generated.contains("ForEachField("));
    EXPECT_TRUE(result.generated.contains("ForEachMember("));
    EXPECT_TRUE(result.generated.contains("ForEachBase("));
    EXPECT_TRUE(result.generated.contains("GetBaseNumbers()"));
    EXPECT_TRUE(
        result.generated.contains("std::integral_constant<decltype(&Model::value), "
                                  "&Model::value>{}, RNoSetter{}"));
    EXPECT_TRUE(result.generated.contains(".meta = {}"));
}

TEST(ClassReflectorMetaTests, EmitsSetterThunkForSetterMeta)
{
    const auto result = Generate(ClassWithField("FIELD(R::Meta(Setter = setValue));"));

    ASSERT_TRUE(result.succeeded);
    EXPECT_EQ(result.errors, 0);
    EXPECT_EQ(result.warnings, 0);
    EXPECT_TRUE(result.generated.contains("obj.setValue(value);"));
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Setter\", \"setValue\" }"));
}

TEST(ClassReflectorMetaTests, ResolvesMetaStringWhenItIsTheFirstStringInTheFile)
{
    const auto result = Generate(ClassWithField("FIELD(R::Meta(DisplayName = \"First string\"));"));

    ASSERT_TRUE(result.succeeded);
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"DisplayName\", \"First string\" }"));
}

TEST(ClassReflectorMetaTests, ResolvesMetaStringsAfterOtherStringAndCharLiterals)
{
    const auto result = Generate(R"(#pragma once
#include "Some/Header.h"
#include "Another/Header.h"
CLASS();
class Model
{
    R_FRIEND(Model);
    FIELD();
    char letter = 'x';
    FIELD();
    char other = 'y';
    FIELD();
    std::string text = "before";
    FIELD(R::Meta(Tooltip = "Second", Label = "", Category = "Third one"));
    int value = 0;
    FIELD(R::Meta(Tooltip = "Fourth"));
    int last = 0;
};
)");

    ASSERT_TRUE(result.succeeded);
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Tooltip\", \"Second\" }"));
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Label\", \"\" }"));
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Category\", \"Third one\" }"));
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Tooltip\", \"Fourth\" }"));
}

TEST(ClassReflectorMetaTests, AcceptsQualifiedIdentifierAndNumericValues)
{
    const auto result
        = Generate(ClassWithField("FIELD(R::Meta(Editor = Gui::AssetRef, Order = -12, Hidden));"));

    ASSERT_TRUE(result.succeeded);
    EXPECT_EQ(result.warnings, 0);
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Editor\", \"Gui::AssetRef\" }"));
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Order\", \"-12\" }"));
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Hidden\", \"\" }"));
}

TEST(ClassReflectorMetaTests, WarnsAndKeepsLastValueForDuplicateMetaKey)
{
    const auto result = Generate(ClassWithField("FIELD(R::Meta(Order = 1), R::Meta(Order = 2));"));

    EXPECT_TRUE(result.succeeded);
    EXPECT_EQ(result.errors, 0);
    EXPECT_EQ(result.warnings, 1);
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Order\", \"2\" }"));
    EXPECT_FALSE(result.generated.contains("RMetaEntry{ \"Order\", \"1\" }"));
}

TEST(ClassReflectorMetaTests, ReportsErrorForMetaWithoutParentheses)
{
    const auto result = Generate(ClassWithField("FIELD(R::Meta);"));

    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.errors, 1);
}

TEST(ClassReflectorMetaTests, ReportsErrorForMetaEntryWithoutKey)
{
    const auto result = Generate(ClassWithField("FIELD(R::Meta(= 5, Valid = 1));"));

    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.errors, 1);
    // Parsing recovers at the next entry.
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Valid\", \"1\" }"));
}

TEST(ClassReflectorMetaTests, ReportsErrorForMetaEntryWithMissingValue)
{
    const auto result = Generate(ClassWithField("FIELD(R::Meta(Key = , Valid));"));

    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.errors, 1);
    EXPECT_TRUE(result.generated.contains("RMetaEntry{ \"Valid\", \"\" }"));
}

TEST(ClassReflectorMetaTests, ReportsErrorForSetterWithoutValue)
{
    const auto result = Generate(ClassWithField("FIELD(R::Meta(Setter));"));

    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.errors, 1);
    EXPECT_FALSE(result.generated.contains("obj.Setter"));
}

TEST(ClassReflectorMetaTests, ReportsErrorForSetterWithNonIdentifierValue)
{
    const auto result = Generate(ClassWithField("FIELD(R::Meta(Setter = \"setValue\"));"));

    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.errors, 1);
    EXPECT_FALSE(result.generated.contains("obj.setValue(value);"));
}

TEST(ClassReflectorMetaTests, ExposesCombinedFlagsIncludingGeneratorOnlyFlags)
{
    const auto result = Generate(ClassWithField("FIELD(R::NoDefaultValue, R::SuppressLimits);"));

    ASSERT_TRUE(result.succeeded);
    const auto expected = RPoint::Default | RPoint::NoDefaultValue | RPoint::SuppressLimits;
    EXPECT_TRUE(result.generated.contains(".flags = " + std::to_string(expected)));
}
