#include "model.h"

#include <iostream>
#include <type_traits>

int main()
{
    // tag::enum_metadata[]
    std::cout << R<Priority>::FullName() << " has " << R<Priority>::Size() << " values\n";
    std::cout << R<Priority>::ToString(Priority::High) << '\n';

    if (const auto value = R<Priority>::FromString("Normal"))
    {
        std::cout << static_cast<int>(*value) << '\n';
    }
    // end::enum_metadata[]

    Task task;
    task.id = 7;
    task.title = "Write documentation";
    task.priority = Priority::High;

    // tag::class_metadata[]
    std::cout << R<Task>::FullName() << '\n';
    for (const RClassField& field : R<Task>::GetFields())
    {
        std::cout << field.type << ' ' << field.name << '\n';
    }
    // end::class_metadata[]

    // tag::json_round_trip[]
    const auto stream = R<Task>::Serialize(task);
    std::cout << stream.getData().dump(2) << '\n';

    Task restored;
    R<Task>::Deserialize(stream, restored);
    // end::json_round_trip[]

    // tag::dynamic_field_access[]
    const bool found = R<Task>::GetField(restored, "title",
                                         [](void* address, const char* type)
                                         {
                                             if (std::string_view(type) == "std::string")
                                             {
                                                 *static_cast<std::string*>(address) = "Published";
                                             }
                                         });
    // end::dynamic_field_access[]

    // tag::field_visitor[]
    R<Task>::ForEachField(restored,
                          [](const RClassField& field, const auto& value)
                          {
                              // `value` has the field's real type: std::string, then Priority.
                              using Value = std::remove_cvref_t<decltype(value)>;
                              if constexpr (std::is_same_v<Value, std::string>)
                              {
                                  std::cout << field.name << " = " << value << '\n';
                              }
                          });
    // end::field_visitor[]

    // tag::base_visitor[]
    R<Task>::ForEachBase(
        [](auto base)
        {
            using Base = typename decltype(base)::type;
            std::cout << R<Task>::Name() << " derives from " << R<Base>::Name() << '\n';
        });
    // end::base_visitor[]

    // tag::member_visitor[]
    Lamp lamp;
    R<Lamp>::ForEachMember(
        [&](const RClassField& field, auto member, auto setter)
        {
            const auto label = field.attributes.getMeta("DisplayName").value_or(field.name);
            std::cout << label << " = " << (lamp.*decltype(member)::value) << '\n';

            if constexpr (!std::is_same_v<decltype(setter), RNoSetter>)
            {
                setter(lamp, 2.f); // Calls Lamp::setBrightness, which clamps the value to 1.
            }
        });
    // end::member_visitor[]

    return found && lamp.brightness == 1.f ? 0 : 1;
}
