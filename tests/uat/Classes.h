/*
 * MIT License
 *
 * Copyright (c) 2018-2027 Valerii Koniushenko
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
class Human
{
    R_FRIEND(Human);

public:
    Human(int age, std::string_view name)
        : _age(age),
          _name(name)
    {
    }

private:
    FIELD();
    int _age = 0;

    FIELD();
    std::string _name;
};

namespace NS
{
    CLASS();
    class Car final
    {
        R_FRIEND(Car);

    public:
        enum class EngineType : unsigned char
        {
            Petrol,
            Diesel,
            Electric,
            Hybrid
        };

        struct Spec
        {
            unsigned horsepower{};
            double torqueNm{};
            constexpr Spec(unsigned hp = 0u, double tq = 0.0) noexcept
                : horsepower{ hp },
                  torqueNm{ tq }
            {
            }
        };

    public:
        static inline unsigned s_globalIdCounter = 0;

        FIELD();
        int i = 1;

        FIELD();
        int a;

        FIELD();
        int b = { 123 };

        FIELD();
        int c{ 666 };

        FIELD(R::NoDefaultValue);
        const unsigned _id{ ++s_globalIdCounter };

        FIELD();
        std::string _brand{ "Unknown" };

        FIELD();
        std::string _model{ "Undefined" };

        FIELD();
        EngineType _engine{ NS::Car::EngineType::Petrol };

        FIELD();
        char ch = 'b';

        FIELD();
        Spec _spec{ 100u, 150.0 };

        FIELD();
        std::array<double, 4> _tirePressure{ 2.2, 2.2, 2.2, 2.2 };

        std::unique_ptr<int> _diagnosticCode{};

        bool _running : 1 { false };
        bool _hasError : 1 { false };

    public:
        constexpr static double MaxSpeedLimit = 320.0;

        Car() = default;

        explicit Car(std::string brand, std::string model)
            : _brand(std::move(brand)),
              _model(std::move(model))
        {
        }

        Car(std::string brand, std::string model, EngineType engine, Spec spec) noexcept
            : Car(std::move(brand), std::move(model))
        {
            _engine = engine;
            _spec = spec;
        }

        Car(const Car&) = delete;
        Car& operator=(const Car&) = delete;

        Car(Car&&) noexcept = delete;
        Car& operator=(Car&&) noexcept = delete;

        ~Car() = default;

    public:
        [[nodiscard]]
        auto id() const noexcept -> unsigned
        {
            return _id;
        }

        [[nodiscard]]
        auto fullName() const& -> std::string
        {
            return _brand + " " + _model;
        }

        [[nodiscard]]
        auto fullName() && -> std::string
        {
            return std::move(_brand) + " " + std::move(_model);
        }

        void start() noexcept
        {
            if (!_hasError)
            {
                _running = true;
            }
        }

        void stop() noexcept { _running = false; }

        void setDiagnostic(int code)
        {
            _diagnosticCode = std::make_unique<int>(code);
            _hasError = true;
        }

        void clearDiagnostic() noexcept
        {
            _diagnosticCode.reset();
            _hasError = false;
        }

        [[nodiscard]]
        constexpr bool isRunning() const noexcept
        {
            return _running;
        }

        [[nodiscard]]
        constexpr EngineType engineType() const noexcept
        {
            return _engine;
        }

        void inflateTire(std::size_t index, double value)
        {
            if (index < _tirePressure.size())
            {
                _tirePressure[index] = value;
            }
        }

        friend std::ostream& operator<<(std::ostream& os, const Car& car)
        {
            return os << "Car{id=" << car._id << ", name=" << car._brand << " " << car._model
                      << ", hp=" << car._spec.horsepower << ", running=" << car._running << "}";
        }
    };
} // namespace NS

namespace nlohmann
{
    inline void to_json(json& j, const NS::Car::EngineType& value)
    {
        switch (value)
        {
            case NS::Car::EngineType::Petrol:
                j = "Petrol";
                break;
            case NS::Car::EngineType::Diesel:
                j = "Diesel";
                break;
            case NS::Car::EngineType::Electric:
                j = "Electric";
                break;
            case NS::Car::EngineType::Hybrid:
                j = "Hybrid";
                break;
        }
    }

    inline void from_json(const json& j, NS::Car::EngineType& value)
    {
        const auto s = j.get<std::string>();
        if (s == "Petrol")
        {
            value = NS::Car::EngineType::Petrol;
            return;
        }
        if (s == "Diesel")
        {
            value = NS::Car::EngineType::Diesel;
            return;
        }
        if (s == "Electric")
        {
            value = NS::Car::EngineType::Electric;
            return;
        }
        if (s == "Hybrid")
        {
            value = NS::Car::EngineType::Hybrid;
            return;
        }

        throw std::runtime_error("Unknown NS::Car::EngineType value: " + s);
    }

    inline void to_json(json& j, const NS::Car::Spec& value)
    {
        j = json{
            { "horsepower", value.horsepower },
            { "torqueNm", value.torqueNm },
        };
    }

    inline void from_json(const json& j, NS::Car::Spec& value)
    {
        value.horsepower = j.value("horsepower", 0u);
        value.torqueNm = j.value("torqueNm", 0.0);
    }
} // namespace nlohmann

#include "Classes.generated.h" // added by the code generator. Better don't move it.
