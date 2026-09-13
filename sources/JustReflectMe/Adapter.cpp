// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "Adapter.h"

namespace RInternal
{
    [[nodiscard]] std::unordered_map<std::string, RClassField> GetClassFieldsAsMap(
        const std::vector<RClassField>& fields)
    {
        std::unordered_map<std::string, RClassField> map;
        for (const auto& field : fields)
        {
            map[std::string(field.name)] = field;
        }
        return map;
    }
} // namespace RInternal