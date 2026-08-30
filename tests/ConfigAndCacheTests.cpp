#include "JustReflectMe/Cache.h"
#include "JustReflectMe/Config.h"
#include "TestSupport.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <chrono>
#include <filesystem>
#include <string>

namespace
{
    constexpr std::string_view customConfig = R"(excludedPaths:
  - generated
parsableFileExtensions:
  - .hpp
showEveryIteratedFilePath: true
showSkippedFiles: true
alwaysDirtyCache: true
insertCodeAtTheTop: "// top"
insertCodeAtTheBottom: "// bottom"
ignoreSerializationSignals: false
rFriendAliases:
  - CUSTOM_FRIEND
)";
}

TEST(ConfigManagerTests, SpawnsFallbackConfigAndLoadsDefaults)
{
    TestSupport::TemporaryDirectory project;
    bool hasError = false;
    JRM::ConfigManager manager;

    const auto config = manager.initializeProjectAndLoadConfig(project.path(), hasError);

    EXPECT_FALSE(hasError);
    EXPECT_TRUE(std::filesystem::exists(project.path() / ".jrm" / "config.yaml"));
    EXPECT_EQ(config.getParams().size(), 9);
    EXPECT_TRUE(config.excludedPaths->value.contains("build"));
    EXPECT_THAT(config.parsableFileExtensions->value,
                testing::ElementsAre(".h", ".hpp", ".hxx", ".hh", ".h++"));
    EXPECT_TRUE(
        JRM::ConfigManager::spawnFallbackConfigAsString().contains("alwaysDirtyCache: false"));
}

TEST(ConfigManagerTests, LoadsEverySupportedConfigurationValue)
{
    TestSupport::TemporaryDirectory project;
    (void)project.writeFile(".jrm/config.yaml", std::string(customConfig));
    bool hasError = false;
    JRM::ConfigManager manager;

    const auto config = manager.initializeProjectAndLoadConfig(project.path(), hasError);

    EXPECT_FALSE(hasError);
    EXPECT_THAT(config.excludedPaths->value, testing::ElementsAre("generated"));
    EXPECT_THAT(config.parsableFileExtensions->value, testing::ElementsAre(".hpp"));
    EXPECT_TRUE(config.showEveryIteratedFilePath->value);
    EXPECT_TRUE(config.showSkippedFiles->value);
    EXPECT_TRUE(config.alwaysDirtyCache->value);
    EXPECT_EQ(config.insertCodeAtTheTop->value, "// top");
    EXPECT_EQ(config.insertCodeAtTheBottom->value, "// bottom");
    EXPECT_FALSE(config.ignoreSerializationSignals->value);
    EXPECT_THAT(config.rFriendAliases->value, testing::ElementsAre("CUSTOM_FRIEND"));
}

TEST(ConfigManagerTests, ReportsMalformedYaml)
{
    TestSupport::TemporaryDirectory project;
    (void)project.writeFile(".jrm/config.yaml", "showEveryIteratedFilePath: \"unterminated\n");
    bool hasError = false;

    const auto config
        = JRM::ConfigManager().initializeProjectAndLoadConfig(project.path(), hasError);

    EXPECT_TRUE(hasError);
    EXPECT_FALSE(config.showEveryIteratedFilePath->value);
}

TEST(ConfigManagerTests, ReportsUnknownTopLevelFields)
{
    TestSupport::TemporaryDirectory project;
    (void)project.writeFile(".jrm/config.yaml", "unknownField: true\n");
    bool hasError = false;

    testing::internal::CaptureStderr();
    (void)JRM::ConfigManager().initializeProjectAndLoadConfig(project.path(), hasError);
    const auto error = testing::internal::GetCapturedStderr();

    EXPECT_FALSE(hasError);
    EXPECT_THAT(error, testing::HasSubstr("Unknown fields in the config file: 'unknownField'"));
}

TEST(ConfigManagerTests, HandlesFallbackConfigCreationFailures)
{
    TestSupport::TemporaryDirectory project;
    (void)project.writeFile(".jrm", "not a directory\n");
    bool hasError = false;

    const auto config
        = JRM::ConfigManager().initializeProjectAndLoadConfig(project.path(), hasError);

    EXPECT_FALSE(hasError);
    EXPECT_FALSE(config.showSkippedFiles->value);
}

TEST(CacheTests, PersistsRelativeFileTimestampsAndHonorsIgnoreMode)
{
    TestSupport::TemporaryDirectory project;
    const auto source = project.writeFile("include/value.h", "#pragma once\n");
    const auto timestamp = std::filesystem::last_write_time(source);

    {
        JRM::Cache cache(project.path(), false);
        EXPECT_TRUE(cache.isNeedUpdate("include/value.h", timestamp));
        cache.updateFile(source);
        cache.saveCache();
    }

    {
        JRM::Cache cache(project.path(), false);
        EXPECT_FALSE(cache.isNeedUpdate("include/value.h", timestamp));
    }

    {
        JRM::Cache cache(project.path(), true);
        EXPECT_TRUE(cache.isNeedUpdate("include/value.h", timestamp));
        cache.updateFile(source);
    }

    EXPECT_TRUE(std::filesystem::exists(project.path() / ".jrm" / "cache.data"));
}

TEST(CacheTests, HandlesMissingProjectsAndCacheFiles)
{
    TestSupport::TemporaryDirectory project;
    const auto missingProject = project.path() / "missing";

    {
        JRM::Cache cache({}, false);
    }
    {
        JRM::Cache cache(missingProject, false);
    }
    {
        JRM::Cache cache(project.path(), false);
    }
    {
        JRM::Cache cache(project.path(), true);
    }

    EXPECT_TRUE(std::filesystem::exists(project.path() / ".jrm" / "cache.data"));
}

TEST(CacheTests, HandlesRelativeAbsoluteAndChangedFileTimes)
{
    TestSupport::TemporaryDirectory project;
    const auto source = project.writeFile("src/value.h", "#pragma once\n");
    const auto timestamp = std::filesystem::last_write_time(source);

    JRM::Cache cache(project.path(), false);
    EXPECT_TRUE(cache.isNeedUpdate("src/value.h"));
    EXPECT_TRUE(cache.isNeedUpdate("src/value.h", timestamp));
    cache.updateFile(source);
    EXPECT_FALSE(cache.isNeedUpdate("src/value.h"));
    EXPECT_FALSE(cache.isNeedUpdate("src/value.h", timestamp));
    EXPECT_TRUE(cache.isNeedUpdate("src/value.h", timestamp + std::chrono::seconds(1)));
    cache.ignoreAnyCacheRequestAndSave(true);
    EXPECT_TRUE(cache.isNeedUpdate("src/value.h", timestamp));
    cache.updateFile(source);
    cache.ignoreAnyCacheRequestAndSave(false);
    cache.updateFile("src/value.h");
    cache.saveCache();

    JRM::Cache loaded(project.path(), false);
    EXPECT_FALSE(loaded.isNeedUpdate("src/value.h", timestamp));
}

TEST(CacheTests, ReadsAbsolutePathsAndEmptyLines)
{
    TestSupport::TemporaryDirectory project;
    const auto first = project.writeFile("first.h", "first");
    const auto second = project.writeFile("second.h", "second");
    const auto time = std::filesystem::file_time_type{ std::chrono::nanoseconds{ 123 } };
    const auto ticks = time.time_since_epoch().count();
    const auto cacheText = std::to_string(ticks) + " " + first.generic_string() + "\n\n"
                           + std::to_string(ticks) + " " + second.generic_string();
    (void)project.writeFile(".jrm/cache.data", cacheText);

    JRM::Cache cache(project.path(), false);

    EXPECT_FALSE(cache.isNeedUpdate("first.h", time));
    EXPECT_FALSE(cache.isNeedUpdate("second.h", time));
}

TEST(CacheTests, IgnoresCorruptedCacheEntries)
{
    TestSupport::TemporaryDirectory project;
    (void)project.writeFile(".jrm/cache.data", "corrupted");

    JRM::Cache cache(project.path(), false);

    EXPECT_TRUE(cache.isNeedUpdate("value.h", std::filesystem::file_time_type{}));
}

TEST(CacheTests, IgnoresEmptyPathsWhenWritingCache)
{
    TestSupport::TemporaryDirectory project;
    (void)project.writeFile(".jrm/cache.data", "123 \n");

    {
        JRM::Cache cache(project.path(), false);
    }

    EXPECT_TRUE(std::filesystem::exists(project.path() / ".jrm" / "cache.data"));
}
