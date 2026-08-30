#include "model.h"

#include <iostream>

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

    return found ? 0 : 1;
}
