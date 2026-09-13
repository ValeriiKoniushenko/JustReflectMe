// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once
#include "FileProcessor.h"
#include "Scopes.h"

#include <string>

namespace JRM
{

    /**
     * @brief Holds the preprocessed contents and scopes of one source file.
     *
     * String and character literals are replaced by placeholders during preprocessing; their
     * original values remain available through the corresponding lookup functions.
     */
    class FileData
    {
    public:
        FileData() = default;
        FileData(const FileData&) = delete;
        FileData& operator=(const FileData&) = delete;
        FileData(FileData&&) noexcept = default;
        FileData& operator=(FileData&&) noexcept = default;
        virtual ~FileData() = default;

        /**
         * @brief Replaces the preprocessed file content and invalidates the scope tree.
         * @param content The preprocessed content to store.
         */
        void setContent(PostProcessedFile&& content);

        /**
         * @brief Returns the preprocessed file text.
         * @return The content used by the reflectors.
         */
        [[nodiscard]] const std::string& getContent() const noexcept;

        /**
         * @brief Returns the scopes currently associated with the file.
         * @return The file's scope tree.
         */
        [[nodiscard]] const Scopes& getScopes() const noexcept;

        /**
         * @brief Replaces the scope tree and marks the cached scopes dirty.
         * @param scope The scope tree to store.
         */
        void setScope(Scopes&& scope);

        /**
         * @brief Returns the source path associated with this file.
         * @return The source path, or an empty string when no path was set.
         */
        [[nodiscard]] const std::string& getPath() const noexcept;

        /**
         * @brief Associates a source path with the file data.
         * @param path The source path.
         */
        void setPath(const std::string& path);

        /**
         * @brief Scans the preprocessed content into a scope tree when needed.
         */
        void scanScopes();

        /**
         * @brief Restores the string literal at a preprocessing placeholder position.
         * @param pos Position of the placeholder in the preprocessed content.
         * @return The original string literal, or an empty string when no token exists there.
         */
        [[nodiscard]] std::string getRealStringFromPlaceholderPos(std::size_t pos) const;

        /**
         * @brief Restores the character literal at a preprocessing placeholder position.
         * @param pos Position of the placeholder in the preprocessed content.
         * @return The original character literal, or an empty string when no token exists there.
         */
        [[nodiscard]] std::string getRealCharFromPlaceholderPos(std::size_t pos) const;

    protected:
        PostProcessedFile _content;
        Scopes _scopes;
        bool _isDirtyScopes = true;
        std::string _path;
    };

} // namespace JRM
