// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "JustReflectMe/Config.h"
#include "JustReflectMe/FileProcessor.h"
#include "JustReflectMe/Reflectors/EnumClassReflector.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace
{

    struct MockFileProcessor : public JRM::FileProcessor
    {
        using JRM::FileProcessor::FileProcessor;

        MOCK_METHOD(void, onPreGenerateContent, (const std::string& content), (const, override));
        MOCK_METHOD(void, onPostGenerateHeaderContent, (const std::string& content),
                    (const, override));
    };

    class FileProcessorTests : public testing::Test
    {
    public:
        struct RAIIFile
        {
            RAIIFile(const std::string& filename_, const std::string& content)
            {
                filename = filename_;

                std::ofstream out(filename);
                if (!out.is_open())
                {
                    throw std::runtime_error("Cannot open file: " + filename);
                }

                out.write(content.c_str(), content.size() * sizeof(char));
            }
            ~RAIIFile() { release(); }

            void release()
            {
                std::error_code ec;
                fs::remove(filename.c_str(), ec);

                auto path = fs::path(filename);
                auto originalExt = path.extension().generic_string();
                if (originalExt == ".cpp" || originalExt == ".h")
                {
                    const auto originalPath = path;
                    path.replace_extension(".generated.h");
                    fs::remove(path, ec);

                    path = originalPath;
                    path.replace_extension(".generated.inl");
                    fs::remove(path, ec);

                    path = originalPath;
                    path.replace_extension(".generated.cpp");
                    fs::remove(path, ec);
                }
            }

            [[nodiscard]] const std::string& getFilename() const { return filename; }
            [[nodiscard]] operator const std::string&() const { return filename; }
            [[nodiscard]] operator fs::path() const { return fs::path(filename); }

        private:
            std::string filename;
        };

        MockFileProcessor processor;

    public:
        FileProcessorTests() = default;
        ~FileProcessorTests() override = default;

        void SetUp() override {}
        void TearDown() override {}
    };

} // namespace

TEST(FileProcessorRegistrationTests, RejectsDuplicateReflectors)
{
    JRM::FileProcessor processor;
    processor.registerReflector<JRM::EnumClassReflector>();
    const auto* original = processor.getReflectors().front().get();
    processor.registerReflector<JRM::EnumClassReflector>();

    ASSERT_EQ(processor.getReflectors().size(), 1U);
    EXPECT_EQ(processor.getReflectors().front().get(), original);
}

TEST_F(FileProcessorTests, SingleFile)
{
    const RAIIFile file("test.cpp", R"(/* some file */
#pragma once        // 2 line
                    // 3
ENUM_CLASS          // 4
enum class TestEnum // 5
{                   // 6
    Hello,          // 7
    World           // 8
};                  // 9
                    // 10
std::string hello = "world"; // 11
char hello = 'c';   // 12
std::string hello = "world" "ggggg" "ssss""aaaa"; // 13
std::string hello = "world" "ddddd" // 14
                    "ssss""aaaa"; // 15
//16
//// ================= MY FILE!!! ===================== //17
//18
std::string sss = "////////"; // 19

6 / 2 = 3;//21
)");

    processor.registerReflector<JRM::EnumClassReflector>();

    EXPECT_CALL(processor, onPreGenerateContent(testing::_))
        .WillOnce(
            [&](const std::string& content)
            {
                ASSERT_FALSE(content.contains("#include \"test.generated.inl\""));
                //
            });

    EXPECT_CALL(processor, onPostGenerateHeaderContent(testing::_))
        .WillOnce(
            [&](const std::string& content)
            {
                ASSERT_TRUE(content.contains("#include \"test.generated.h\""));
                //
            });

    JRM::Config dummy;

    (void)processor.run(file, dummy);
}

TEST_F(FileProcessorTests, FindEnumClassAtNamespace)
{
    const RAIIFile sources("test_1.cpp", R"(#include "test_1.h")");
    const RAIIFile header("test_1.h", R"(#pragma once
namespace NS
{
    namespace NS1{
        class TestClass{};
    }

    namespace NS222{

        namespace NS1{
            class TestClass{};
        }

        namespace NS222_333{

            ENUM_CLASS
            enum class TestEnum
            {
                Hello,
                World
            };
        }
    }

    namespace NS3{
        class TestClass{};
    }
} // namespace NS

)");

    EXPECT_CALL(processor, onPreGenerateContent(testing::_))
        .WillOnce(
            [&](const std::string& content)
            {
                ASSERT_FALSE(content.contains("#include \"test_1.generated.inl\""));
                //
            });

    EXPECT_CALL(processor, onPostGenerateHeaderContent(testing::_))
        .WillOnce(
            [&](const std::string& content)
            {
                ASSERT_TRUE(content.contains("#include \"test_1.generated.h\""));
                //
            });

    processor.registerReflector<JRM::EnumClassReflector>();

    JRM::Config dummy;

    (void)processor.run(header, dummy);
}
