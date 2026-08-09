/*
 * MIT License
 *
 * Copyright (c) 2018-2027 Valerii Koniushenko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Enums.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

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
    ASSERT_EQ("Red", R<Color>::Serialize(Color::Red));
    ASSERT_EQ("Hello", R<Foo::TestEnum>::Serialize(Foo::TestEnum::Hello));

    std::string out;

    R<Color>::Serialize(out, Color::Red);
    ASSERT_EQ("Red", out);

    R<Foo::TestEnum>::Serialize(out, Foo::TestEnum::Hello);
    ASSERT_EQ("Hello", out);
}

TEST(Enums, Deserialize)
{
    Color out;
    R<Color>::Deserialize("Red", out);
    ASSERT_EQ(Color::Red, out);

    Foo::TestEnum out2;
    R<Foo::TestEnum>::Deserialize("Hello", out2);
    ASSERT_EQ(Foo::TestEnum::Hello, out2);
}