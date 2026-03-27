

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

#pragma once
#include "FileProcessor.h"
#include "Scopes.h"

#include <string>

namespace JRM
{

    class FileData
    {
    public:
        FileData() = default;
        FileData(const FileData&) = delete;
        FileData& operator=(const FileData&) = delete;
        FileData(FileData&&) noexcept = default;
        FileData& operator=(FileData&&) noexcept = default;
        virtual ~FileData() = default;

        void setContent(PostProcessedFile&& content);
        [[nodiscard]] const std::string& getContent() const noexcept;

        [[nodiscard]] const Scopes& getScopes() const noexcept;
        void setScope(Scopes&& scope);

        [[nodiscard]] const std::string& getPath() const noexcept;
        void setPath(const std::string& path);

        void scanScopes();
        [[nodiscard]] std::string getRealStringFromPlaceholderPos(std::size_t pos) const;
        [[nodiscard]] std::string getRealCharFromPlaceholderPos(std::size_t pos) const;

    protected:
        PostProcessedFile _content;
        Scopes _scopes;
        std::string _path;
    };

} // namespace JRM
