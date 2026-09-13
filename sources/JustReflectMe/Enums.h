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

namespace JRM
{

    /**
     * @brief Identifies the C++ context represented by a scanned scope.
     */
    enum class ContextType
    {
        Undefined,
        File,
        Namespace,
        EnumClass,
        Class,
        Struct
    };

    /**
     * @brief Converts a context type to its human-readable name.
     * @param type The context type to convert.
     * @return The lower-case display name, or `"undefined"` for an unknown value.
     */
    [[nodiscard]] std::string ToString(ContextType type);

} // namespace JRM
