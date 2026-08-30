#pragma once

#include "JustReflectMe/Adapter.h"

#include <string>

// tag::reflected_enum[]
ENUM_CLASS();
enum class Priority
{
    Low,
    Normal,
    High
};
// end::reflected_enum[]

// tag::reflected_classes[]
CLASS();
class Entity
{
    R_FRIEND(Entity);

public:
    FIELD(R::Required, R::Attr = Identity);
    int id = 0;
};

CLASS();
class Task : public Entity
{
    R_FRIEND(Task, Entity);

public:
    FIELD();
    std::string title = "Untitled";

    FIELD();
    Priority priority = Priority::Normal;
};
// end::reflected_classes[]

// tag::generated_include[]
#include "model.generated.h" // Added automatically by jrm after the first scan.
// end::generated_include[]
