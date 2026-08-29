#include "JustReflectMe/Config.h"
#include "JustReflectMe/FileProcessor.h"
#include "JustReflectMe/Reflectors/ClassReflector.h"
#include "JustReflectMe/Reflectors/EnumClassReflector.h"

#include "TestSupport.h"
#include "gtest/gtest.h"

TEST(GenerationPipelineTests, GeneratesAndIntegratesClassAndEnumReflection)
{
    TestSupport::TemporaryDirectory project;
    const auto header = project.writeFile("include/DomainTypes.h", R"(#pragma once
namespace Domain
{
ENUM_CLASS();
enum class Kind { First, Explicit = 42 };

CLASS();
class Widget
{
    R_FRIEND(Widget);
    FIELD(R::Attr = Tone);
    int amount = 7;
    FIELD(R::NoDefaultValue);
    std::string label{"name"};
};
}
)");

    JRM::Config config;
    config.insertCodeAtTheTop->value = "// generated top";
    config.insertCodeAtTheBottom->value = "// generated bottom";

    JRM::FileProcessor processor;
    processor.registerReflector<JRM::EnumClassReflector>();
    processor.registerReflector<JRM::ClassReflector>();

    ASSERT_TRUE(processor.run(header, config));

    const auto generated = project.readFile("include/DomainTypes.generated.h");
    const auto integratedHeader = project.readFile("include/DomainTypes.h");

    EXPECT_TRUE(generated.contains("// generated top"));
    EXPECT_TRUE(generated.contains("// generated bottom"));
    EXPECT_TRUE(generated.contains("R<Domain::Kind>"));
    EXPECT_TRUE(generated.contains("R<Domain::Widget>"));
    EXPECT_TRUE(generated.contains("amount"));
    EXPECT_TRUE(generated.contains("label"));
    EXPECT_TRUE(generated.contains("#include <string>"));
    EXPECT_TRUE(integratedHeader.contains("#include \"DomainTypes.generated.h\""));

    ASSERT_TRUE(processor.run(header, config));
    const auto afterSecondRun = project.readFile("include/DomainTypes.h");
    const auto include = std::string_view("#include \"DomainTypes.generated.h\"");
    EXPECT_EQ(afterSecondRun.find(include), afterSecondRun.rfind(include));
    EXPECT_EQ(project.readFile("include/DomainTypes.generated.h"), generated);
}

TEST(GenerationPipelineTests, LeavesFilesWithoutReflectionMarkersUntouched)
{
    TestSupport::TemporaryDirectory project;
    const auto header = project.writeFile("Plain.h", "#pragma once\nclass Plain {};\n");
    JRM::Config config;
    JRM::FileProcessor processor;
    processor.registerReflector<JRM::EnumClassReflector>();
    processor.registerReflector<JRM::ClassReflector>();

    EXPECT_TRUE(processor.run(header, config));
    EXPECT_FALSE(std::filesystem::exists(project.path() / "Plain.generated.h"));
    EXPECT_EQ(project.readFile("Plain.h"), "#pragma once\nclass Plain {};\n");
}
