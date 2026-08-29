#include "JustReflectMe/Cache.h"
#include "JustReflectMe/Config.h"

#include "TestSupport.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

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
    EXPECT_TRUE(JRM::ConfigManager::spawnFallbackConfigAsString().contains("alwaysDirtyCache: false"));
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
