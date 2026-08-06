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

#include "Classes.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(Classes, FullNames)
{
    ASSERT_EQ(R<Human>::FullName(), "Human");
    ASSERT_EQ(R<NS::Car>::FullName(), "NS::Car");
}

TEST(Classes, Names)
{
    ASSERT_EQ(R<Human>::Name(), "Human");
    ASSERT_EQ(R<NS::Car>::Name(), "Car");
}

TEST(Classes, ParentScopes)
{
    ASSERT_EQ(R<Human>::ParentScope(), "");
    ASSERT_EQ(R<NS::Car>::ParentScope(), "NS");
}

TEST(Classes, FieldNumbers)
{
    ASSERT_EQ(R<Human>::GetFieldNumbers(), 2);
    ASSERT_EQ(R<NS::Car>::GetFieldNumbers(), 11);
}

TEST(Classes, FieldNumberss_Human)
{
    const auto fields = R<Human>::GetFieldsMap();
    ASSERT_EQ(fields.size(), 2);
    ASSERT_TRUE(fields.contains("_age"));
    ASSERT_TRUE(fields.contains("_name"));

    ASSERT_EQ(fields.find("_age")->second.name, "_age");
    ASSERT_EQ(fields.find("_age")->second.type, "int");
    ASSERT_EQ(fields.find("_name")->second.name, "_name");
    ASSERT_EQ(fields.find("_name")->second.type, "std::string");
}