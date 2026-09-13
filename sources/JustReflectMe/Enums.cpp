// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "Enums.h"

namespace JRM
{

    std::string ToString(ContextType type)
    {
        if (type == ContextType::Undefined)
        {
            return "undefined";
        }
        if (type == ContextType::File)
        {
            return "file";
        }
        if (type == ContextType::Namespace)
        {
            return "namespace";
        }
        if (type == ContextType::EnumClass)
        {
            return "enum class";
        }
        if (type == ContextType::Class)
        {
            return "class";
        }
        if (type == ContextType::Struct)
        {
            return "struct";
        }

        return "undefined";
    }
} // namespace JRM
