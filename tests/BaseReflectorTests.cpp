#define JRM_ENABLE_TESTS
#include "JustReflectMe/Config.h"
#include "JustReflectMe/FileData.h"
#include "JustReflectMe/FileProcessor.h"
#include "JustReflectMe/Reflectors/BaseReflector.h"
#undef JRM_ENABLE_TESTS

#include "gtest/gtest.h"
#include <array>
#include <limits>
#include <optional>
#include <string>

namespace
{

    class TestReflector final : public JRM::BaseReflector
    {
    public:
        explicit TestReflector(bool supportsImplTranslationUnit = false)
        {
            _isSupportImplTranslationUnit = supportsImplTranslationUnit;
        }

        constexpr std::string_view getTriggerKeyword() const noexcept override { return "TRIGGER"; }

        std::optional<JRM::TypeMeta> findKnownTypeMeta(const std::string& fullPath) const override
        {
            if (fullPath == "known")
            {
                return JRM::TypeMeta{ JRM::ContextType::Class };
            }
            return std::nullopt;
        }

        void postScanCrossLinksResolving() override { ++postScanCalls; }

        void exposeWarn(const std::string& source, std::size_t index)
        {
            warnMessage(source.c_str(), index, "file.hpp", "warning message");
        }

        void exposeError(const std::string& source, std::size_t index)
        {
            errorMessage(source.c_str(), index, "file.hpp", "error message");
        }

        static std::size_t exposeFindTriggerKeyword(const std::string& content,
                                                    std::string_view keyword, std::size_t offset)
        {
            return findTriggerKeyword(content, keyword, offset);
        }

        static std::string exposePrettyPrintScope(const JRM::Scope* scope)
        {
            return PrettyPrintScope(scope);
        }

        static std::string exposePrettyPrintIdentifier(const JRM::Scope* scope)
        {
            return PrettyPrintIdentifier(scope);
        }

        static void exposeRequireValidToken(std::size_t begin, const std::string& content)
        {
            TokenEntry token{ begin };
            token.requireValidTokenBasedOnContent(content);
        }

        [[nodiscard]] std::size_t tokenCount() const noexcept { return _tokens.size(); }

        static std::string exposeFullNamePath(const std::string& name,
                                              const std::string& parentSpace)
        {
            BaseTokenData data;
            data.name = name;
            data.parentSpace = parentSpace;
            return data.fullNamePath();
        }

        std::optional<JRM::TypeMeta> exposeFindGloballyKnownTypeMeta(
            const std::string& fullPath) const
        {
            return findGloballyKnownTypeMeta(fullPath);
        }

        void exposeSetConfig(const JRM::Config& config) { setConfig(config); }

        std::string exposeGenerateHeader(JRM::FileData& data) const
        {
            return generateHeaderFile(data);
        }

        std::string exposeGenerateSource(JRM::FileData& data) const
        {
            return generateSourceFile("unused", data);
        }

        void exposeSetHasImplTranslationUnit(bool value) { setHasImplTranslationUnit(value); }

        int postScanCalls = 0;
        int scanCalls = 0;

    protected:
        std::string onGenerateHeaderFile(JRM::FileData&) const override { return headerResult; }

        std::string onGenerateSourceFile(JRM::FileData&) const override { return sourceResult; }

        void onScan(const JRM::FileData&) override { ++scanCalls; }

    public:
        std::string headerResult;
        std::string sourceResult;
    };

    JRM::FileData fileDataWithContent(std::string content)
    {
        JRM::FileData data;
        JRM::PostProcessedFile processed;
        processed.content = std::move(content);
        data.setContent(std::move(processed));
        return data;
    }

    JRM::Scope namedScope(const char* identifier, JRM::Scope* parent = nullptr)
    {
        JRM::Scope scope;
        scope.identifierStart = identifier;
        scope.start = identifier;
        scope.end = identifier + std::char_traits<char>::length(identifier);
        scope.parent = parent;
        scope.type = JRM::ContextType::Class;
        return scope;
    }

} // namespace

TEST(BaseReflectorTests, ConvertsSeverityAndTracksMessages)
{
    TestReflector reflector;

    EXPECT_EQ(TestReflector::SeverityToString(JRM::BaseReflector::Severity::Warning), "warning");
    EXPECT_EQ(TestReflector::SeverityToString(JRM::BaseReflector::Severity::Error), "error");
    // NOLINTNEXTLINE(bugprone-invalid-enum-cast) - exercise the defensive default branch.
    EXPECT_EQ(TestReflector::SeverityToString(static_cast<JRM::BaseReflector::Severity>(99)),
              "unknown");
    EXPECT_FALSE(reflector.hasWarnings());
    EXPECT_FALSE(reflector.hasErrors());
    EXPECT_EQ(reflector.numberOfWarnings(), 0);
    EXPECT_EQ(reflector.numberOfErrors(), 0);

    reflector.exposeWarn("source", 1);
    reflector.exposeError("source", 2);
    reflector.exposeWarn("source", 3);

    EXPECT_TRUE(reflector.hasWarnings());
    EXPECT_TRUE(reflector.hasErrors());
    EXPECT_EQ(reflector.numberOfWarnings(), 2);
    EXPECT_EQ(reflector.numberOfErrors(), 1);
    EXPECT_EQ(reflector.numberOfSeverity(JRM::BaseReflector::Severity::Warning), 2);
    EXPECT_EQ(reflector.numberOfSeverity(JRM::BaseReflector::Severity::Error), 1);
    // NOLINTNEXTLINE(bugprone-invalid-enum-cast) - exercise an absent severity entry.
    EXPECT_EQ(reflector.numberOfSeverity(static_cast<JRM::BaseReflector::Severity>(99)), 0);
}

TEST(BaseReflectorTests, FormatsSyntaxExceptionWithLocation)
{
    JRM::SyntaxException exception("unexpected token", 3);

    EXPECT_EQ(exception.what(), std::string("unexpected token"));
    EXPECT_EQ(exception.getFullMessage("ab\ncd", "source.hpp"),
              "source.hpp:2:1: error: unexpected token");
}

TEST(BaseReflectorTests, DetectsTriggerWordsAndScansOnlyWhenPresent)
{
    TestReflector reflector;
    EXPECT_FALSE(reflector.canProcessContent("nothing"));
    EXPECT_FALSE(reflector.canProcessContent("TRIGGER2 "));
    EXPECT_FALSE(reflector.canProcessContent("TRIGGER"));
    EXPECT_TRUE(reflector.canProcessContent("TRIGGER; "));

    auto noTrigger = fileDataWithContent("not a trigger");
    reflector.scanContent(noTrigger);
    EXPECT_FALSE(reflector.hasTokens());
    EXPECT_EQ(reflector.scanCalls, 0);

    auto data = fileDataWithContent("XTRIGGER TRIGGER; TRIGGER, TRIGGER");
    reflector.scanContent(data);
    EXPECT_TRUE(reflector.hasTokens());
    EXPECT_EQ(reflector.scanCalls, 1);
    EXPECT_EQ(reflector.tokenCount(), 3);
}

TEST(BaseReflectorTests, FindsWholeKeywordOccurrencesFromOffset)
{
    EXPECT_EQ(TestReflector::exposeFindTriggerKeyword("XTRIGGER TRIGGER TRIGGER2", "TRIGGER", 0),
              9);
    EXPECT_EQ(TestReflector::exposeFindTriggerKeyword("XTRIGGER TRIGGER TRIGGER2", "TRIGGER", 10),
              std::string::npos);
    EXPECT_EQ(TestReflector::exposeFindTriggerKeyword("nothing", "TRIGGER", 0), std::string::npos);
}

TEST(BaseReflectorTests, GeneratesHeadersAndSourcesWithSingleTrailingNewline)
{
    auto data = fileDataWithContent("");
    TestReflector reflector;

    EXPECT_TRUE(reflector.exposeGenerateHeader(data).empty());
    EXPECT_TRUE(reflector.exposeGenerateSource(data).empty());

    reflector.headerResult = "header";
    reflector.sourceResult = "source";
    EXPECT_EQ(reflector.exposeGenerateHeader(data), "header\n");
    EXPECT_EQ(reflector.exposeGenerateSource(data), "source\n");
    reflector.sourceResult = "source\n";
    EXPECT_EQ(reflector.exposeGenerateSource(data), "source\n");
    EXPECT_EQ(reflector.getIncludes(), std::set<std::string>{ "string" });
}

TEST(BaseReflectorTests, HandlesImplementationSupportAndConfiguration)
{
    TestReflector unsupported;
    unsupported.exposeSetHasImplTranslationUnit(true);
    EXPECT_FALSE(unsupported.isSupportImplTranslationUnit());
    EXPECT_FALSE(unsupported.hasImplTranslationUnit());

    TestReflector supported(true);
    EXPECT_TRUE(supported.isSupportImplTranslationUnit());
    EXPECT_FALSE(supported.hasImplTranslationUnit());
    supported.exposeSetHasImplTranslationUnit(true);
    EXPECT_TRUE(supported.hasImplTranslationUnit());
    supported.exposeSetHasImplTranslationUnit(false);
    EXPECT_FALSE(supported.hasImplTranslationUnit());

    JRM::Config config;
    supported.exposeSetConfig(config);
}

TEST(BaseReflectorTests, ValidatesTokensAndBuildsFullNames)
{
    const std::string content = "token";
    TestReflector::exposeRequireValidToken(2, content);

    EXPECT_THROW(
        TestReflector::exposeRequireValidToken(std::numeric_limits<std::size_t>::max(), content),
        std::runtime_error);

    EXPECT_THROW(TestReflector::exposeRequireValidToken(content.size(), content),
                 std::runtime_error);

    EXPECT_EQ(TestReflector::exposeFullNamePath("Name", ""), "Name");
    EXPECT_EQ(TestReflector::exposeFullNamePath("Name", "Outer::Inner"), "Outer::Inner::Name");
}

TEST(BaseReflectorTests, PrintsScopeIdentifiersAndFindsGlobalTypes)
{
    const std::array outerName{ 'O', 'u', 't', 'e', 'r', '\0' };
    const std::array innerName{ 'I', 'n', 'n', 'e', 'r', '\0' };
    JRM::Scope root;
    auto outer = namedScope(outerName.data(), &root);
    auto inner = namedScope(innerName.data(), &outer);

    EXPECT_EQ(TestReflector::exposePrettyPrintScope(nullptr), "");
    EXPECT_EQ(TestReflector::exposePrettyPrintScope(&outer), "Outer");
    EXPECT_EQ(TestReflector::exposePrettyPrintScope(&inner), "Outer::Inner");
    EXPECT_EQ(TestReflector::exposePrettyPrintIdentifier(&inner), "class Outer::Inner");

    JRM::FileProcessor processor;
    processor.registerReflector<TestReflector>();
    TestReflector reflector;
    reflector.setParentFileProcessor(&processor);
    EXPECT_FALSE(reflector.exposeFindGloballyKnownTypeMeta("missing"));
    EXPECT_TRUE(reflector.exposeFindGloballyKnownTypeMeta("known"));
}
