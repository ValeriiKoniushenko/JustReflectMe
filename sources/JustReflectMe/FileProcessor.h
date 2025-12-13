

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

#pragma once

#include "Reflectors/BaseReflector.h"

#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#if defined(JRM_ENABLE_TESTS)
    #include "gtest/gtest_prod.h"
#endif

namespace JRM
{

    class FileProcessor
    {
    public:
        FileProcessor() = default;
        FileProcessor(const FileProcessor&) = delete;
        FileProcessor& operator=(const FileProcessor&) = delete;
        FileProcessor(FileProcessor&&) noexcept = delete;
        FileProcessor& operator=(FileProcessor&&) noexcept = delete;
        virtual ~FileProcessor() = default;

        void setFilePath(const std::string& path);
        void run();

        template<typename T>
            requires std::is_base_of_v<BaseReflector, T>
        void registerReflector()
        {
#if defined(NDEBUG)
            if (hasReflector<T>())
            {
                std::cerr << "Such a reflector '" << typeid(T).name() << "' already registered!" << std::endl;
                return;
            }
#endif

            _reflectors.emplace_back(std::make_unique<T>());
        }
        template<typename T>
            requires std::is_base_of_v<BaseReflector, T>
        [[nodiscard]] bool hasReflector()
        {
            for (auto&& reflector : _reflectors)
            {
                if (typeid(*reflector) == typeid(T))
                {
                    return true;
                }
            }

            return false;
        }

    private:
        struct TokenEntry final
        {
            static constexpr std::size_t invalidPosition = std::numeric_limits<std::size_t>::max();
            std::size_t begin = invalidPosition;
            std::size_t end = invalidPosition;

            std::size_t processableReflectorTypeHash = 0;

            [[nodiscard]] bool isValid() const noexcept;
        };

    private:
        [[nodiscard]] std::vector<TokenEntry> findAllEntryPoints() const;

    protected:
        std::vector<std::unique_ptr<BaseReflector>> _reflectors;
        std::string _filePath;

#if defined(JRM_ENABLE_TESTS)
        FRIEND_TEST(FileProcessorTests, FindAllEntryPoints);
#endif
    };

} // namespace JRM
