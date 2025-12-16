/*
 * MIT License
 *
 * Copyright (c) 2018-2025 Valerii Koniushenko
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

#include "JustReflectMe/FileProcessor.h"
#include "JustReflectMe/Reflectors/EnumClassReflector.h"

#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

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

    private:
        std::string filename;
    };

    JRM::FileProcessor processor;

public:
    FileProcessorTests() = default;
    ~FileProcessorTests() override = default;

    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(FileProcessorTests, FindAllEntryPoints)
{
    const RAIIFile file("test.cpp", R"(
#pragma once        // 2 line
                    // 3
ENUM_CLASS          // 4
enum class TestEnum // 5
{                   // 6
    Hello,          // 7
    World           // 8
};                  // 9
)");

    processor.registerReflector<JRM::EnumClassReflector>();
    processor.setFilePath(file);

    processor.run();
}