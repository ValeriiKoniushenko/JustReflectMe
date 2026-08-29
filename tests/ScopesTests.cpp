#include "JustReflectMe/Scopes.h"

#include "gtest/gtest.h"

TEST(ScopesTests, ResolvesNestedNamespaceTemplateClassAndEnumScopes)
{
    const std::string content = R"(namespace Outer
{
template<class T>
class Holder
{
    enum class State { Idle, Ready };
};
})";

    JRM::Scopes scopes;
    scopes.scan(content);

    const auto* namespaceScope = scopes.getScopeAt(content.c_str() + content.find("template"));
    ASSERT_NE(namespaceScope, nullptr);
    EXPECT_EQ(namespaceScope->type, JRM::ContextType::Namespace);
    EXPECT_EQ(namespaceScope->getIdentifier(), "Outer");

    const auto* classScope = scopes.getScopeAt(content.c_str() + content.find("enum class"));
    ASSERT_NE(classScope, nullptr);
    EXPECT_EQ(classScope->type, JRM::ContextType::Class);
    EXPECT_EQ(classScope->getIdentifier(), "Holder");
    EXPECT_EQ(classScope->attribute, JRM::Scope::Attr_Template);

    const auto* enumScope = scopes.getScopeAt(content.c_str() + content.find("Idle"));
    ASSERT_NE(enumScope, nullptr);
    EXPECT_EQ(enumScope->type, JRM::ContextType::EnumClass);
    EXPECT_EQ(enumScope->getIdentifier(), "State");
}

TEST(ScopesTests, RejectsEmptyContentAndReturnsNullOutsideTheScannedFile)
{
    JRM::Scopes scopes;
    EXPECT_THROW(scopes.scan(""), std::runtime_error);

    const std::string content = "struct Value {};";
    scopes.scan(content);
    EXPECT_EQ(scopes.getScopeAt(content.c_str() + content.size()), nullptr);
}
