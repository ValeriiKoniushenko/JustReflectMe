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
#include <uat/Enums.h>

TEST(Enums, FullNames)
{
    ASSERT_EQ(R<Color>::FullName(), "Color");
    ASSERT_EQ(R<Foo::TestEnum>::FullName(), "Foo::TestEnum");
}

TEST(Enums, Names)
{
    ASSERT_EQ(R<Color>::Name(), "Color");
    ASSERT_EQ(R<Foo::TestEnum>::Name(), "TestEnum");
}

TEST(Enums, ParentScopes)
{
    ASSERT_EQ(R<Color>::ParentScope(), "");
    ASSERT_EQ(R<Foo::TestEnum>::ParentScope(), "Foo");
}

TEST(Enums, Size)
{
    ASSERT_EQ(R<Color>::Size(), 3);
    ASSERT_EQ(R<Foo::TestEnum>::Size(), 2);
}

TEST(Enums, ToArrayOfContstants)
{
    constexpr auto array = R<Color>::ToArrayC();
    ASSERT_EQ(array.size(), 3);
    ASSERT_EQ(array[0], Color::Red);
    ASSERT_EQ(array[1], Color::Green);
    ASSERT_EQ(array[2], Color::Blue);

    // ===========================

    constexpr auto array2 = R<Foo::TestEnum>::ToArrayC();
    ASSERT_EQ(array2.size(), 2);
    ASSERT_EQ(array2[0], Foo::TestEnum::Hello);
    ASSERT_EQ(array2[1], Foo::TestEnum::World);
}

TEST(Enums, ToArrayOfNames)
{
    constexpr auto array = R<Color>::ToArrayN();
    ASSERT_EQ(array.size(), 3);
    ASSERT_EQ(array[0], "Red");
    ASSERT_EQ(array[1], "Green");
    ASSERT_EQ(array[2], "Blue");

    // ===========================

    constexpr auto array2 = R<Foo::TestEnum>::ToArrayN();
    ASSERT_EQ(array2.size(), 2);
    ASSERT_EQ(array2[0], "Hello");
    ASSERT_EQ(array2[1], "World");
}

TEST(Enums, ToString)
{
    ASSERT_EQ(R<Color>::ToString(Color::Red), "Red");
    ASSERT_EQ(R<Color>::ToString(Color::Green), "Green");
    ASSERT_EQ(R<Color>::ToString(Color::Blue), "Blue");

    // ===========================

    ASSERT_EQ(R<Foo::TestEnum>::ToString(Foo::TestEnum::Hello), "Hello");
    ASSERT_EQ(R<Foo::TestEnum>::ToString(Foo::TestEnum::World), "World");
}

TEST(Enums, FromString)
{
    ASSERT_TRUE(R<Color>::FromString("Red").has_value());
    ASSERT_TRUE(R<Color>::FromString("Green").has_value());
    ASSERT_TRUE(R<Color>::FromString("Blue").has_value());

    ASSERT_FALSE(R<Color>::FromString("red").has_value());
    ASSERT_FALSE(R<Color>::FromString("green").has_value());
    ASSERT_FALSE(R<Color>::FromString("blue").has_value());

    ASSERT_EQ(Color::Red, R<Color>::FromString("Red").value());
    ASSERT_EQ(Color::Green, R<Color>::FromString("Green").value());
    ASSERT_EQ(Color::Blue, R<Color>::FromString("Blue").value());

    // ===========================

    ASSERT_TRUE(R<Foo::TestEnum>::FromString("Hello").has_value());
    ASSERT_TRUE(R<Foo::TestEnum>::FromString("World").has_value());
    ASSERT_FALSE(R<Foo::TestEnum>::FromString("hello").has_value());
    ASSERT_FALSE(R<Foo::TestEnum>::FromString("world").has_value());

    ASSERT_EQ(Foo::TestEnum::Hello, R<Foo::TestEnum>::FromString("Hello").value());
    ASSERT_EQ(Foo::TestEnum::World, R<Foo::TestEnum>::FromString("World").value());
}

TEST(Enums, Serialize)
{
    {
        RResourceStream<RJsonResourceStream> s;
        R<Color>::Serialize(Color::Blue, s);
        ASSERT_EQ("Blue", s.getData().get<std::string>());
    }

    {
        RResourceStream<RJsonResourceStream> s;
        R<Foo::TestEnum>::Serialize(Foo::TestEnum::Hello, s);
        ASSERT_EQ("Hello", s.getData().get<std::string>());
    }
}

TEST(Enums, Deserialize)
{
    {
        RResourceStream<RJsonResourceStream> s(nlohmann::json::string_t{ "Blue" });
        Color out{ Color::Red };
        R<Color>::Deserialize(s, out);
        ASSERT_EQ(Color::Blue, out);
    }

    {
        RResourceStream<RJsonResourceStream> s(nlohmann::json::string_t{ "Hello" });
        Foo::TestEnum out2{ Foo::TestEnum::Hello };
        R<Foo::TestEnum>::Deserialize(s, out2);
        ASSERT_EQ(Foo::TestEnum::Hello, out2);
    }
}
