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
    FIELD();
    std::string _radio = "0.0.0.0";
};
#include "header.generated.inl" // added by the code generator. Better don't move it.
