// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <string>
#include <type_traits>
#include <typeindex>
#include <uat/FieldVisitors.h>
#include <vector>

using testing::ElementsAre;
using UAT::Both;
using UAT::Empty;
using UAT::Left;
using UAT::Light;

namespace
{
    std::vector<std::string> FieldNames(const auto& fields)
    {
        std::vector<std::string> out;
        for (const RClassField& field : fields)
        {
            out.emplace_back(field.name);
        }
        return out;
    }

    // Walks the reflected base chain first, then the class's own fields.
    template<class T>
    void CollectFieldNamesOfWholeChain(T& obj, std::vector<std::string>& out)
    {
        R<T>::ForEachBase(
            [&](auto tag)
            {
                using Base = typename decltype(tag)::type;
                CollectFieldNamesOfWholeChain<Base>(static_cast<Base&>(obj), out);
            });
        R<T>::ForEachField(obj, [&](const RClassField& field, auto&)
                           { out.emplace_back(field.name); });
    }
} // namespace

TEST(FieldVisitors, GetFieldsArrayKeepsDeclarationOrder)
{
    const auto& fields = R<Light>::GetFieldsArray();

    static_assert(std::tuple_size_v<std::remove_cvref_t<decltype(fields)>> == 4);
    EXPECT_THAT(FieldNames(fields), ElementsAre("_intensity", "name", "id", "values"));
    EXPECT_EQ(fields[0].type, "float");
    EXPECT_EQ(fields[1].type, "std::string");
    EXPECT_EQ(fields[2].type, "const int");
    EXPECT_EQ(fields[3].type, "std::vector<int>");
}

TEST(FieldVisitors, GetFieldsArrayIsCachedAndMatchesGetFields)
{
    EXPECT_EQ(&R<Light>::GetFieldsArray(), &R<Light>::GetFieldsArray());
    EXPECT_EQ(FieldNames(R<Light>::GetFieldsArray()), FieldNames(R<Light>::GetFields()));
}

TEST(FieldVisitors, ForEachFieldVisitsInDeclarationOrderWithTypedReferences)
{
    Light light;
    std::vector<std::string> names;
    std::vector<std::type_index> types;

    R<Light>::ForEachField(light,
                           [&](const RClassField& field, auto& value)
                           {
                               names.emplace_back(field.name);
                               types.emplace_back(typeid(value));
                           });

    EXPECT_THAT(names, ElementsAre("_intensity", "name", "id", "values"));
    EXPECT_THAT(types, ElementsAre(std::type_index(typeid(float)),
                                   std::type_index(typeid(std::string)),
                                   std::type_index(typeid(int)),
                                   std::type_index(typeid(std::vector<int>))));
}

TEST(FieldVisitors, ForEachFieldPassesTheSameMetadataObjectsAsGetFieldsArray)
{
    Light light;
    std::vector<const RClassField*> visited;

    R<Light>::ForEachField(light, [&](const RClassField& field, auto&)
                           { visited.push_back(&field); });

    const auto& fields = R<Light>::GetFieldsArray();
    EXPECT_THAT(visited, ElementsAre(&fields[0], &fields[1], &fields[2], &fields[3]));
}

TEST(FieldVisitors, ForEachFieldWritesThroughReferencesWithoutCallingSetters)
{
    Light light;

    R<Light>::ForEachField(light,
                           [](const RClassField&, auto& value)
                           {
                               using V = std::remove_reference_t<decltype(value)>;
                               if constexpr (std::is_same_v<V, float>)
                               {
                                   value = 4.f;
                               }
                               else if constexpr (std::is_same_v<V, std::string>)
                               {
                                   value = "Sun";
                               }
                               else if constexpr (std::is_same_v<V, std::vector<int>>)
                               {
                                   value.push_back(9);
                               }
                           });

    EXPECT_EQ(light.getIntensity(), 4.f);
    EXPECT_EQ(light.name, "Sun");
    EXPECT_THAT(light.values, ElementsAre(9));
    EXPECT_EQ(light.setIntensityCalls, 0);
}

TEST(FieldVisitors, ForEachFieldOnConstObjectYieldsConstReferences)
{
    const Light light;
    int visited = 0;

    R<Light>::ForEachField(light,
                           [&](const RClassField&, auto& value)
                           {
                               static_assert(
                                   std::is_const_v<std::remove_reference_t<decltype(value)>>);
                               ++visited;
                           });

    EXPECT_EQ(visited, 4);
}

TEST(FieldVisitors, ForEachFieldKeepsConstFieldsConstOnMutableObject)
{
    Light light;
    std::vector<bool> constness;

    R<Light>::ForEachField(light, [&](const RClassField&, auto& value)
                           { constness.push_back(std::is_const_v<
                                                 std::remove_reference_t<decltype(value)>>); });

    EXPECT_THAT(constness, ElementsAre(false, false, true, false));
}

TEST(FieldVisitors, ForEachMemberProvidesCompileTimeMemberPointers)
{
    Light light;
    light.name = "Desk lamp";
    std::vector<std::string> names;
    std::string observedName;

    R<Light>::ForEachMember(
        [&](const RClassField& field, auto member, auto)
        {
            constexpr auto pointer = decltype(member)::value;
            static_assert(std::is_member_object_pointer_v<decltype(pointer)>);

            names.emplace_back(field.name);
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(light.*pointer)>,
                                         std::string>)
            {
                observedName = light.*pointer;
            }
        });

    EXPECT_THAT(names, ElementsAre("_intensity", "name", "id", "values"));
    EXPECT_EQ(observedName, "Desk lamp");
}

TEST(FieldVisitors, ForEachMemberProvidesGeneratedSetterOnlyWhenDeclared)
{
    Light light;
    std::vector<bool> hasSetter;

    R<Light>::ForEachMember(
        [&](const RClassField&, auto, auto setter)
        {
            using Setter = decltype(setter);
            hasSetter.push_back(!std::is_same_v<Setter, RNoSetter>);
            if constexpr (!std::is_same_v<Setter, RNoSetter>)
            {
                // Captureless: usable as a plain function pointer by type-erased consumers.
                void (*asFunctionPointer)(Light&, const float&) = setter;
                asFunctionPointer(light, 6.f);
            }
        });

    EXPECT_THAT(hasSetter, ElementsAre(true, false, false, false));
    EXPECT_EQ(light.getIntensity(), 6.f);
    EXPECT_EQ(light.setIntensityCalls, 1);
}

TEST(FieldVisitors, VisitorsDoNothingForClassWithoutFieldsOrBases)
{
    Empty empty;
    int calls = 0;

    R<Empty>::ForEachField(empty, [&](const RClassField&, auto&) { ++calls; });
    R<Empty>::ForEachMember([&](const RClassField&, auto, auto) { ++calls; });
    R<Empty>::ForEachBase([&](auto) { ++calls; });

    EXPECT_EQ(calls, 0);
    EXPECT_TRUE(R<Empty>::GetFieldsArray().empty());
    EXPECT_EQ(R<Empty>::GetBaseNumbers(), 0);
}

TEST(FieldVisitors, ForEachBaseFollowsRFriendOrderAndSkipsUnreflectedBases)
{
    std::vector<std::string_view> bases;

    R<Both>::ForEachBase(
        [&](auto tag)
        {
            using Base = typename decltype(tag)::type;
            bases.push_back(R<Base>::FullName());
        });

    EXPECT_THAT(bases, ElementsAre("UAT::Left", "UAT::Right"));
    EXPECT_EQ(R<Both>::GetBaseNumbers(), 2);
    EXPECT_EQ(R<Left>::GetBaseNumbers(), 0);
}

TEST(FieldVisitors, BaseAndFieldVisitorsComposeIntoAWholeChainWalk)
{
    Both both;
    std::vector<std::string> names;

    CollectFieldNamesOfWholeChain(both, names);

    EXPECT_THAT(names, ElementsAre("left", "right", "both"));
}

TEST(FieldVisitors, ExposesFieldFlags)
{
    const auto& fields = R<Light>::GetFieldsArray();

    const auto& intensity = fields[0].attributes;
    EXPECT_TRUE(intensity.hasFlag(RPoint::Min));
    EXPECT_TRUE(intensity.hasFlag(RPoint::Max));
    EXPECT_TRUE(intensity.hasFlag(RPoint::Step));
    EXPECT_TRUE(intensity.hasFlag(RPoint::Meta));
    EXPECT_FALSE(intensity.hasFlag(RPoint::Required));
    EXPECT_FALSE(intensity.hasFlag(RPoint::SuppressLimits));

    const auto& name = fields[1].attributes;
    EXPECT_TRUE(name.hasFlag(RPoint::Required));
    EXPECT_TRUE(name.hasFlag(RPoint::Meta));

    const auto& id = fields[2].attributes;
    EXPECT_TRUE(id.hasFlag(RPoint::NoDefaultValue));
    EXPECT_TRUE(id.hasFlag(RPoint::SuppressLimits));
    EXPECT_TRUE(id.hasFlag(RPoint::Min));
    EXPECT_EQ(std::get<int64_t>(*id.min), -1);

    EXPECT_EQ(fields[3].attributes.flags, RPoint::Default);
}

TEST(FieldVisitors, ExposesMetaEntriesInDeclarationOrderAcrossMetaGroups)
{
    const auto& fields = R<Light>::GetFieldsArray();

    EXPECT_THAT(fields[0].attributes.meta,
                ElementsAre(RMetaEntry{ "DisplayName", "Light intensity" },
                            RMetaEntry{ "Category", "Lighting" },
                            RMetaEntry{ "Setter", "setIntensity" }));
    EXPECT_THAT(fields[1].attributes.meta,
                ElementsAre(RMetaEntry{ "ReadOnly", "" }, RMetaEntry{ "Order", "3" },
                            RMetaEntry{ "Offset", "-2.5" },
                            RMetaEntry{ "Tooltip", "Shown in the scene tree" }));
    EXPECT_TRUE(fields[2].attributes.meta.empty());
    EXPECT_TRUE(fields[3].attributes.meta.empty());
}

TEST(FieldVisitors, MetaLookupDistinguishesBareKeysFromMissingKeys)
{
    const auto& name = R<Light>::GetFieldsArray()[1].attributes;

    EXPECT_TRUE(name.hasMeta("ReadOnly"));
    EXPECT_EQ(name.getMeta("ReadOnly"), std::optional<std::string_view>(""));
    EXPECT_EQ(name.getMeta("Order"), std::optional<std::string_view>("3"));
    EXPECT_FALSE(name.hasMeta("Missing"));
    EXPECT_EQ(name.getMeta("Missing"), std::nullopt);
}
