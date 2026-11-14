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

#include <string>
#include <vector>

namespace UAT
{
    CLASS();
    class Light
    {
        R_FRIEND(Light);

    public:
        void setIntensity(float value)
        {
            _intensity = value;
            ++setIntensityCalls;
        }

        // An overload must not make the generated setter call ambiguous.
        void setIntensity(int value) { setIntensity(static_cast<float>(value)); }

        [[nodiscard]] float getIntensity() const noexcept { return _intensity; }

        int setIntensityCalls = 0;

    protected:
        FIELD(R::Min = 0.0, R::Max = 10.0, R::Step = 0.5,
              R::Meta(DisplayName = "Light intensity", Category = Lighting,
                      Setter = setIntensity));
        float _intensity = 1.f;

    public:
        FIELD(R::Required, R::Meta(ReadOnly, Order = 3, Offset = -2.5),
              R::Meta(Tooltip = "Shown in the scene tree"));
        std::string name = "Lamp";

        FIELD(R::NoDefaultValue, R::SuppressLimits, R::Min = -1);
        const int id = 7;

        FIELD();
        std::vector<int> values;
    };

    CLASS();
    struct Empty
    {
        R_FRIEND(Empty);
    };

    CLASS();
    class Left
    {
        R_FRIEND(Left);

    public:
        FIELD();
        int left = 1;
    };

    CLASS();
    class Right
    {
        R_FRIEND(Right);

    public:
        FIELD();
        int right = 2;
    };

    struct NotReflected
    {
        int ignored = 0;
    };

    CLASS();
    class Both : public Left, public NotReflected, public Right
    {
        R_FRIEND(Both, UAT::Left, UAT::Right);

    public:
        FIELD();
        int both = 3;
    };
} // namespace UAT
