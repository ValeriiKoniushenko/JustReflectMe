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

#include "header.h"

#include <iostream>
using namespace std;

#define ASSERT_EQ(a, b)                                                                            \
    if (a != b)                                                                                    \
    {                                                                                              \
        std::cerr << "Expected equality, but:" << std::endl;                                       \
        std::cerr << #a << " != " << #b << std::endl;                                              \
        return 1;                                                                                  \
    }

int main()
{
    ASSERT_EQ(R<Color>::Name(), "Color");
    ASSERT_EQ(R<Color>::Size(), 3);
    ASSERT_EQ(R<Color>::ToString(Color::Red), "Red");
    ASSERT_EQ(R<Color>::FromString("Red"), Color::Red);

    const auto c = R<Color>::ToArrayC();
    ASSERT_EQ(c.size(), 3);
    ASSERT_EQ(c[0], Color::Red);
    ASSERT_EQ(c[1], Color::Green);
    ASSERT_EQ(c[2], Color::Blue);

    const auto n = R<Color>::ToArrayN();
    ASSERT_EQ(n.size(), 3);
    ASSERT_EQ(n[0], "Red");
    ASSERT_EQ(n[1], "Green");
    ASSERT_EQ(n[2], "Blue");

    ASSERT_EQ(R::Color::ParentScope(), "");

    return 0;
}