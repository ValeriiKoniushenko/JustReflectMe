#define JRM_ENABLE_TESTS
#include "JustReflectMe/Scopes.h"
#undef JRM_ENABLE_TESTS

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

TEST(ScopesTests, ScopeValueOperationsAndTreeTraversalHandleAllBoundaries)
{
    const std::string content = " value ";
    const auto* begin = content.c_str();

    JRM::Scope scope;
    EXPECT_FALSE(scope.isValid());

    scope.start = begin + 1;
    EXPECT_FALSE(scope.isValid());
    scope.end = begin + 6;
    scope.identifierStart = begin;
    EXPECT_TRUE(scope.isValid());
    EXPECT_EQ(scope.getIdentifier(), "value");
    EXPECT_FALSE(scope.contains(begin));
    EXPECT_TRUE(scope.contains(scope.start));
    EXPECT_TRUE(scope.contains(scope.end - 1));
    EXPECT_FALSE(scope.contains(scope.end));

    const JRM::Scope sameScope = scope;
    EXPECT_EQ(scope, sameScope);

    JRM::Scope differentStart = scope;
    ++differentStart.start;
    EXPECT_NE(scope, differentStart);

    JRM::Scope differentEnd = scope;
    --differentEnd.end;
    EXPECT_NE(scope, differentEnd);

    scope.identifierStart = nullptr;
    EXPECT_TRUE(scope.getIdentifier().empty());

    JRM::Scope root;
    root.start = begin;
    root.end = begin + 6;
    root.children.emplace_back();
    root.children.front().start = begin + 2;
    root.children.front().end = begin + 5;
    root.children.front().children.emplace_back();
    root.children.front().children.front().start = begin + 3;
    root.children.front().children.front().end = begin + 4;
    root.revalidateTree();

    EXPECT_EQ(root.children.front().parent, &root);
    EXPECT_EQ(root.children.front().children.front().parent, &root.children.front());
    EXPECT_EQ(root.findDeepest(begin + 1), &root);
    EXPECT_EQ(root.findDeepest(begin + 3), &root.children.front().children.front());
    EXPECT_EQ(root.findDeepest(begin + 6), nullptr);
}

TEST(ScopesTests, ScansEverySupportedDeclarationKindAndUnknownBlocks)
{
    const std::string content = R"(namespace NS { int namespaceValue; }
enum class Mode { First, Last };
class [[maybe_unused]] Widget { int classValue; };
struct Record { int structValue; };
void functionScope() { int functionValue; }
)";

    JRM::Scopes scopes;
    EXPECT_EQ(scopes.getScopeAt(nullptr), nullptr);
    scopes.scan(content);

    const auto getScopeAt = [&](std::string_view marker)
    { return scopes.getScopeAt(content.c_str() + content.find(marker)); };

    const auto* namespaceScope = getScopeAt("namespaceValue");
    ASSERT_NE(namespaceScope, nullptr);
    EXPECT_EQ(namespaceScope->type, JRM::ContextType::Namespace);
    EXPECT_EQ(namespaceScope->getIdentifier(), "NS");

    const auto* enumScope = getScopeAt("First");
    ASSERT_NE(enumScope, nullptr);
    EXPECT_EQ(enumScope->type, JRM::ContextType::EnumClass);
    EXPECT_EQ(enumScope->getIdentifier(), "Mode");

    const auto* classScope = getScopeAt("classValue");
    ASSERT_NE(classScope, nullptr);
    EXPECT_EQ(classScope->type, JRM::ContextType::Class);
    EXPECT_EQ(classScope->getIdentifier(), "Widget");

    const auto* structScope = getScopeAt("structValue");
    ASSERT_NE(structScope, nullptr);
    EXPECT_EQ(structScope->type, JRM::ContextType::Struct);
    EXPECT_EQ(structScope->getIdentifier(), "Record");

    const auto* unknownScope = getScopeAt("functionValue");
    ASSERT_NE(unknownScope, nullptr);
    EXPECT_EQ(unknownScope->type, JRM::ContextType::Undefined);
    EXPECT_TRUE(unknownScope->getIdentifier().empty());
}

TEST(ScopesTests, ParsesInlineTemplateDeclarations)
{
    const std::string content = "template <typename T> class Inline { T value; };";

    JRM::Scopes scopes;
    scopes.scan(content);

    const auto* scope = scopes.getScopeAt(content.c_str() + content.find("value"));
    ASSERT_NE(scope, nullptr);
    EXPECT_EQ(scope->type, JRM::ContextType::Class);
    EXPECT_EQ(scope->getIdentifier(), "Inline");
    EXPECT_EQ(scope->attribute, JRM::Scope::Attr_Template);
}

TEST(ScopesTests, ScansAnonymousBlocksAtInputStartAndDeclarationsAfterSemicolons)
{
    const std::string anonymousBlock = " {}";
    JRM::Scopes anonymousScopes;
    anonymousScopes.scan(anonymousBlock);

    const auto* anonymousScope = anonymousScopes.getScopeAt(anonymousBlock.c_str() + 1);
    ASSERT_NE(anonymousScope, nullptr);
    EXPECT_EQ(anonymousScope->type, JRM::ContextType::Undefined);

    const std::string semicolonSeparated = "marker; struct Semi { int member; };";
    JRM::Scopes semicolonScopes;
    semicolonScopes.scan(semicolonSeparated);

    const auto* structScope = semicolonScopes.getScopeAt(semicolonSeparated.c_str()
                                                         + semicolonSeparated.find("member"));
    ASSERT_NE(structScope, nullptr);
    EXPECT_EQ(structScope->type, JRM::ContextType::Struct);
    EXPECT_EQ(structScope->getIdentifier(), "Semi");
}

TEST(ScopesTests, RejectsMalformedTemplatesAttributesAndUnbalancedClosingBraces)
{
    JRM::Scopes scopes;

    EXPECT_THROW(scopes.scan("template Broken {"), std::runtime_error);
    EXPECT_THROW(scopes.scan("template <typename T class MissingAngle {"), std::runtime_error);
    EXPECT_THROW(scopes.scan("class [[broken Widget {"), std::runtime_error);
    EXPECT_THROW(scopes.scan("}}"), std::runtime_error);
}

namespace JRM
{
    TEST(ScopesTests, IgnoresNonOpeningScopeStarts)
    {
        constexpr char content[] = "x";
        Scope scope;

        Scopes::tryToDetermineScopeType(scope, content, content);

        EXPECT_EQ(scope.type, ContextType::Undefined);
        EXPECT_EQ(scope.identifierStart, nullptr);
    }
} // namespace JRM
