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
    ASSERT_EQ(R<Animal>::FullName(), "Animal");

    ASSERT_EQ(R<Foo0>::FullName(), "Foo0");
    ASSERT_EQ(R<Foo1>::FullName(), "Foo1");
    ASSERT_EQ(R<Foo2>::FullName(), "Foo2");
    ASSERT_EQ(R<Foo3>::FullName(), "Foo3");
    ASSERT_EQ(R<Foo4>::FullName(), "Foo4");
    ASSERT_EQ(R<Foo5>::FullName(), "Foo5");
    ASSERT_EQ(R<Foo6>::FullName(), "Foo6");
    ASSERT_EQ(R<Foo7>::FullName(), "Foo7");
    // ASSERT_EQ(R<Foo8>::FullName(), "Foo8");
    ASSERT_EQ(R<Foo9>::FullName(), "Foo9");
}

TEST(Classes, Names)
{
    ASSERT_EQ(R<Human>::Name(), "Human");
    ASSERT_EQ(R<NS::Car>::Name(), "Car");
    ASSERT_EQ(R<Animal>::Name(), "Animal");
}

TEST(Classes, ParentScopes)
{
    ASSERT_EQ(R<Human>::ParentScope(), "");
    ASSERT_EQ(R<NS::Car>::ParentScope(), "NS");
    ASSERT_EQ(R<Animal>::ParentScope(), "");
}

TEST(Classes, FieldNumbers)
{
    ASSERT_EQ(R<Human>::GetFieldNumbers(), 2);
    ASSERT_EQ(R<NS::Car>::GetFieldNumbers(), 11);
    ASSERT_EQ(R<Animal>::GetFieldNumbers(), 3);
}

TEST(Classes, FieldMetadata_Human)
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

TEST(Classes, FieldMetadata_Car)
{
    const auto fields = R<NS::Car>::GetFieldsMap();

    ASSERT_EQ(fields.size(), 11);

    ASSERT_TRUE(fields.contains("i"));
    ASSERT_TRUE(fields.contains("a"));
    ASSERT_TRUE(fields.contains("b"));
    ASSERT_TRUE(fields.contains("c"));
    ASSERT_TRUE(fields.contains("_id"));
    ASSERT_TRUE(fields.contains("_brand"));
    ASSERT_TRUE(fields.contains("_model"));
    ASSERT_TRUE(fields.contains("_engine"));
    ASSERT_TRUE(fields.contains("ch"));
    ASSERT_TRUE(fields.contains("_spec"));
    ASSERT_TRUE(fields.contains("_tirePressure"));

    ASSERT_EQ(fields.find("i")->second.name, "i");
    ASSERT_EQ(fields.find("i")->second.type, "int");

    ASSERT_EQ(fields.find("a")->second.name, "a");
    ASSERT_EQ(fields.find("a")->second.type, "int");

    ASSERT_EQ(fields.find("b")->second.name, "b");
    ASSERT_EQ(fields.find("b")->second.type, "int");

    ASSERT_EQ(fields.find("c")->second.name, "c");
    ASSERT_EQ(fields.find("c")->second.type, "int");

    ASSERT_EQ(fields.find("_id")->second.name, "_id");
    ASSERT_EQ(fields.find("_id")->second.type, "const unsigned");

    ASSERT_EQ(fields.find("_brand")->second.name, "_brand");
    ASSERT_EQ(fields.find("_brand")->second.type, "std::string");

    ASSERT_EQ(fields.find("_model")->second.name, "_model");
    ASSERT_EQ(fields.find("_model")->second.type, "std::string");

    ASSERT_EQ(fields.find("_engine")->second.name, "_engine");
    ASSERT_EQ(fields.find("_engine")->second.type, "EngineType");

    ASSERT_EQ(fields.find("ch")->second.name, "ch");
    ASSERT_EQ(fields.find("ch")->second.type, "char");

    ASSERT_EQ(fields.find("_spec")->second.name, "_spec");
    ASSERT_EQ(fields.find("_spec")->second.type, "Spec");

    ASSERT_EQ(fields.find("_tirePressure")->second.name, "_tirePressure");
    ASSERT_EQ(fields.find("_tirePressure")->second.type, "std::array<double, 4>");
}

TEST(Classes, FieldMetadata_Animal)
{
    const auto fields = R<Animal>::GetFieldsMap();

    ASSERT_EQ(fields.size(), 3);
    ASSERT_TRUE(fields.contains("_type"));
    ASSERT_TRUE(fields.contains("_name"));
    ASSERT_TRUE(fields.contains("_age"));

    ASSERT_EQ(fields.find("_type")->second.type, "AnimalType");
    ASSERT_EQ(fields.find("_name")->second.type, "std::string");
    ASSERT_EQ(fields.find("_age")->second.type, "int");
}

TEST(Classes, Serialize_Animal)
{
    Animal animal;
    animal._age = 12;
    animal._name = "Bob";
    animal._type = AnimalType::Cat;

    const auto json = R<Animal>::Serialize(animal).getData();

    ASSERT_TRUE(json.is_object());

    ASSERT_EQ(json.size(), 3);

    ASSERT_TRUE(json.contains("_age"));
    ASSERT_TRUE(json.contains("_name"));
    ASSERT_TRUE(json.contains("_type"));

    EXPECT_EQ(json["_age"], 12);
    EXPECT_EQ(json["_name"], "Bob");
    EXPECT_EQ(json["_type"], R<AnimalType>::ToString(AnimalType::Cat));
}

TEST(Classes, Deserialize_Animal)
{
    nlohmann::json json = { { "_age", 12 },
                            { "_name", "Bob" },
                            { "_type", R<AnimalType>::ToString(AnimalType::Cat) } };

    Animal animal;

    RResourceStream<RJsonResourceStream> stream(json);
    R<Animal>::Deserialize(stream, animal);

    EXPECT_EQ(animal._age, 12);
    EXPECT_EQ(animal._name, "Bob");
    EXPECT_EQ(animal._type, AnimalType::Cat);
}