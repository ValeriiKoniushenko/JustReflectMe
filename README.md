# Just Reflect Me (JRM)

> Fast, user-friendly C++ code reflection — no compiler support required.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)]()
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)]()
[![CMake](https://img.shields.io/badge/build-CMake-informational.svg)]()

JRM is a zero-dependency code reflection library for C++. Connect it to your
project, add two CMake lines, and your `.h` files become fully reflectable —
no compiler patches, no new standards required.

```cpp
// ============== Header.h ==============
#include <JustReflectMe/Adapter.h>

ENUM_CLASS();
enum class Color {
    Red,
    Green,
    Blue
};

CLASS();
class Car {
    R_FRIEND(Car);
    
    FIELD();
    int _speed = 0;
    
    FIELD();
    std::string _model;   
};

// ============== main.cpp ==============
for (auto [name, value] : R<Color>::ToMapNC()) {
    cout << name << " = " << (int)value << endl;
}

Car volvo;
for (auto field : R<Car>::GetFields()) {
    cout << field.type << " " << field.name << endl;
}
auto data = R<Car>::Serialize(volvo); // json by default

```

---

## Why JRM?

C++ still lacks standardized compiler-based reflection. While
[P2996](https://isocpp.org/files/papers/P2996R13.html) has seen its first
commits land in GCC, it remains experimental, GCC-only for now, with no
confirmed timeline for Clang or MSVC. More critically, even once it ships
across all three compilers, it will target only the newest C++ standards —
leaving C++17 and C++20 projects without a solution.

JRM fills that gap today, on any compiler, starting from C++17.

> JRM was created to power [Nexium](https://github.com/vakon/Nexium), an
> open-source game engine with demanding runtime reflection needs. Everything
> built for Nexium is available here for your project too.

---

## Features

- Written in **C++23**
- **CMake**-based build system
- **Zero dependencies** (Google Test is optional, for running the test suite)
- Works on GCC, Clang, and MSVC
- Reflection metadata lives in `.h` files — no separate annotation step

---

## Integration

### 1. Add the repository

Clone JRM into your project:

```bash
git clone https://github.com/ValeriiKoniushenko/JustReflectMe JRM
```

### 2. Register in CMake

```cmake
# Optionally disable JRM's own test suite to speed up your build
set(JRM_DISABLE_TESTS ON)

add_subdirectory(path/to/JRM)
target_link_libraries(YourTarget PUBLIC JustReflectMe::Adapter)
```

### 3. Register the code generator

```cmake
add_custom_target(JRM ALL
    COMMAND jrm ${CMAKE_SOURCE_DIR}
)
add_dependencies(YourTarget JRM)
```

That's it. On the next build, JRM scans your headers and generates reflection
metadata automatically.

---

## Usage

Write reflectable structs inside `.h` files (the code generator only scans
headers by default). More detailed examples of how to use it with code are below.

On first run, JRM creates a `.jrm/` folder in your project root:

| File          | Purpose                                                                |
|---------------|------------------------------------------------------------------------|
| `cache.data`  | Build cache. Delete it to force a full regeneration on the next run.   |
| `config.yaml` | All configurable options, with inline documentation. Open and explore. |

### Enum class reflection

```cpp
// Header.h

#include <JustReflectMe/JustReflectMe.h>

ENUM_CLASS();
enum class Color {
   Red,
   Green,
   Blue
};

// main.cpp
int main(){
   cout << R<Color>::Name() << endl; // > Color
   cout << R<Color>::Size() << endl; // > 3

   cout << R<Color>::ToString(Color::Red) << endl; // > Red
   cout << (int)R<Color>::FromString("Green").value() << endl; // > 0

   std::array names = R<Color>::ToArrayN();
   std::array constants = R<Color>::ToArrayC();
   return 0;
}
```

### Class reflection

More essential example of working with classes and its fields. Nextly, we will work with the next class:

```cpp
// Human.h
#include <JustReflectMe/JustReflectMe.h>

CLASS();
class Human {
    R_FRIEND(Human);
public:
    Human(int age, std::string_view name) 
        : _age(age), _name(name) {}

private:
    FIELD();
    int _age = 0;

    FIELD();
    std::string _name;
};
```

#### Getting of metadata of a class

```cpp
// main.cpp
#include "Human.h"

Human h(25, "Nikki");
cout << R<Human>::Name() << endl;            // Human
cout << R<Human>::GetFieldNumbers() << endl; // 2
for (auto field : R<Human>::GetFields())
{
    cout << field.type << " " << field.name << endl;
}
```

#### De/Serealization of a class

```cpp
// main.cpp
#include "Human.h"

Human h(25, "Nikki");
auto data = R<Human>::Serialize(h); // json by default
auto json = data.getData();
cout << json.dump(4) << endl; // {"age":25,"name":"Nikki"}

Human restored = R<Human>::Deserialize(data);
```

Also, you can extend and change the default serialization format. You should inherit from
`class RBaseResourceStreamImpl` and impelement a few functions: `read` and `write`. The example 'how to' you can find at
the default JSON reflection implementation by the next path: `sources/JustReflectMe/Adapter.h`.

After that, you will be able to use your own serialization system:

```cpp
// main.cpp
#include "Human.h"

Human h(25, "Nikki");
auto data = R<Human>::Serialize<YourOwnResourceStream>(h);
auto someTypeOfYourFormat = data.getData(); 

Human restored = R<Human>::Deserialize<YourOwnResourceStream>(data);
```

---

**Builds**:

- [![MSVC Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FWinBuild_MSVC_Debug%2F&label=MSVC%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/WinBuild_MSVC_Debug/) [![MSVC Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FWinBuild_MSVC_Release%2F&label=MSVC%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/WinBuild_MSVC_Release/)
- [![GCC Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_GCC_Debug%2F&label=GCC%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_GCC_Debug/) [![GCC Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_GCC_Release%2F&label=GCC%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_GCC_Release/)
- [![Clang Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_Clang_Debug%2F&label=Clang%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_Clang_Debug/) [![Clang Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_Clang_Release%2F&label=Clang%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_Clang_Release/)


