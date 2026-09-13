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

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

CLASS();
class Vehicle
{
    R_FRIEND(Vehicle);

public:
    [[nodiscard]] float getSpeed() const noexcept { return _speed; }
    void setSpeed(float v) noexcept { _speed = v; }

protected:
    FIELD();
    float _speed = 0;
};

CLASS();
class Car : public Vehicle
{
    R_FRIEND(Car, Vehicle);

public:
    [[nodiscard]] std::string getName() const noexcept { return _name; }
    void setName(std::string_view v) noexcept { _name = v; }

protected:
    FIELD();
    std::string _name = "None";
};

struct IRadio
{
};

CLASS();
class RadioCar : public IRadio, std::vector<std::unique_ptr<char>>, public Vehicle
{
    R_FRIEND(RadioCar, Vehicle);

public:
    [[nodiscard]] std::string getRadio() const noexcept { return _radio; }
    void setRadio(std::string_view v) noexcept { _radio = v; }

protected:
    // FIELD(A = 1);
    int _connectionFrequency = 50;

    FIELD();
    std::string _radio = "0.0.0.0";
};

#include "MltClasses.generated.h" // added by the code generator. Better don't move it.
