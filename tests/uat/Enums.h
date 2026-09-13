// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "JustReflectMe/Adapter.h"

#define TEST_VALUE_FOR_ENUM_CLASS_TEST 123

ENUM_CLASS();
enum class Color
{
    Red,
    Green,
    Blue
};

namespace Foo
{
    ENUM_CLASS();
    enum class TestEnum
    {
        Hello = 1 << 5,
        World = TEST_VALUE_FOR_ENUM_CLASS_TEST
    };
} // namespace Foo

#include "Enums.generated.h" // added by the code generator. Better don't move it.
