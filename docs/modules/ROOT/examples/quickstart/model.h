#pragma once

#include "JustReflectMe/Adapter.h"

#include <algorithm>
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

// tag::tool_metadata[]
CLASS();
class Lamp
{
    R_FRIEND(Lamp);

public:
    void setBrightness(float value) { brightness = std::clamp(value, 0.f, 1.f); }

    FIELD(R::Min = 0.0, R::Max = 1.0, R::Step = 0.05,
          R::Meta(DisplayName = "Brightness", Category = Lighting, Setter = setBrightness));
    float brightness = 0.5f;

    FIELD(R::Meta(ReadOnly, Tooltip = "Assigned by the scene"));
    int sceneId = 0;
};
// end::tool_metadata[]

// tag::generated_include[]
#include "model.generated.h" // Added automatically by jrm after the first scan.
// end::generated_include[]
