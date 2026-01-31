/*
 * MIT License
 *
 * Copyright (c) 2018-2026 Valerii Koniushenko
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

#include "JustReflectMe/Config.h"
#include "JustReflectMe/FileProcessor.h"
#include "JustReflectMe/Reflectors/EnumClassReflector.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

struct MockFileProcessor : public JRM::FileProcessor
{
    using JRM::FileProcessor::FileProcessor;

    MOCK_METHOD(void, onPreGenerateContent, (const std::string& content), (const, override));
    MOCK_METHOD(void, onPostGenerateHeaderContent, (const std::string& content), (const, override));
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

        void release() { std::filesystem::remove(filename.c_str()); }

        [[nodiscard]] const std::string& getFilename() const { return filename; }
        [[nodiscard]] operator const std::string&() const { return filename; }
        [[nodiscard]] operator std::filesystem::path() const { return std::filesystem::path(filename); }

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
        .WillOnce(testing::Invoke(
            [&](const std::string& content)
            {
                ASSERT_FALSE(content.contains("#include \"test.generated.inl\""));
                //
            }));

    EXPECT_CALL(processor, onPostGenerateHeaderContent(testing::_))
        .WillOnce(testing::Invoke(
            [&](const std::string& content)
            {
                ASSERT_TRUE(content.contains("#include \"test.generated.inl\""));
                //
            }));

    JRM::Config dummy;

    processor.run(file, dummy);
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
    .WillOnce(testing::Invoke(
        [&](const std::string& content)
        {
            ASSERT_FALSE(content.contains("#include \"test_1.generated.inl\""));
            //
        }));

    EXPECT_CALL(processor, onPostGenerateHeaderContent(testing::_))
        .WillOnce(testing::Invoke(
            [&](const std::string& content)
            {
                ASSERT_TRUE(content.contains("#include \"test_1.generated.inl\""));
                //
            }));


    processor.registerReflector<JRM::EnumClassReflector>();

    JRM::Config dummy;

    processor.run(header, dummy);
}