// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <string>
#include <vector>

namespace StringHelper
{
    void FindAndReplaceAll(std::string& str, std::string_view from, const std::string& to);

    void TrimInPlace(std::string& s);

    std::vector<std::string> SplitString(const std::string& str, char delim = ',',
                                         bool isTrim = true);

} // namespace StringHelper