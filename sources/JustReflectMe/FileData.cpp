// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "FileData.h"

namespace JRM
{

    void FileData::setContent(PostProcessedFile&& content)
    {
        _content = std::move(content);
        _isDirtyScopes = true;
    }

    const std::string& FileData::getContent() const noexcept
    {
        return _content.content;
    }

    const Scopes& FileData::getScopes() const noexcept
    {
        return _scopes;
    }

    void FileData::setScope(Scopes&& scope)
    {
        _scopes = std::move(scope);
        _isDirtyScopes = true;
    }

    const std::string& FileData::getPath() const noexcept
    {
        return _path;
    }

    void FileData::setPath(const std::string& path)
    {
        _path = path;
    }

    void FileData::scanScopes()
    {
        if (_isDirtyScopes)
        {
            _scopes.scan(_content.content);
            _isDirtyScopes = false;
        }
    }

    std::string FileData::getRealStringFromPlaceholderPos(std::size_t pos) const
    {
        const auto it = _content.stringTokens.find(pos);
        return it != _content.stringTokens.end() ? it->second : "";
    }

    std::string FileData::getRealCharFromPlaceholderPos(std::size_t pos) const
    {
        const auto it = _content.charTokens.find(pos);
        return it != _content.charTokens.end() ? it->second : "";
    }

} // namespace JRM