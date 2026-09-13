// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "JustReflectMe/JustReflectMe.h"

#include <iostream>
#include <ostream>

int main(int argc, char** argv)
{
    try
    {
        JRM::JustReflectMe obj;
        return obj.run(argc, argv);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[JustReflectMe] Error: " << e.what() << std::endl;
    }

    return 1;
}