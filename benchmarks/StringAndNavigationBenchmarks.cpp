#include "JustReflectMe/FileNavigationHelper.h"
#include "JustReflectMe/StringHelper.h"

#include <benchmark/benchmark.h>
#include <string>

// clang-format off
/*
2026-08-30T11:13:11+02:00
Running build/bin/JRMStringAndNavigationBenchmarks
Run on (16 X 3551.85 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 16384 KiB (x1)
Load Average: 4.27, 2.25, 1.61
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
BM_FindAndReplaceAll/64              50.8 ns         50.6 ns     13737544
BM_FindAndReplaceAll/1024             103 ns          103 ns      6796127
BM_TrimInPlace/64                    34.2 ns         34.1 ns     20562876
BM_TrimInPlace/1024                  44.5 ns         44.4 ns     17013239
BM_SplitString/10                     320 ns          319 ns      2190665
BM_SplitString/100                   2512 ns         2506 ns       281815
BM_GoToLineStart/100                 2.82 ns         2.82 ns    243956349
BM_GoToLineStart/1000                2.76 ns         2.74 ns    245766593
BM_FindFirstWithLineLimit/100        13.3 ns         13.3 ns     52382953
BM_FindFirstWithLineLimit/1000       14.1 ns         14.1 ns     49379010
BM_GetLineNumberAndColumn/100         803 ns          800 ns       867748
BM_GetLineNumberAndColumn/1000       8439 ns         8413 ns        81719
BM_ReadAsTypename                     101 ns          101 ns      6788198
BM_FindScopeEnd/10                   14.3 ns         14.2 ns     52531143
BM_FindScopeEnd/100                   115 ns          115 ns      5947654
*/
// clang-format on

namespace
{

    std::string MakeLines(std::size_t count)
    {
        std::string content;
        content.reserve(count * 24);
        for (std::size_t i = 0; i < count; ++i)
        {
            content += "line ";
            content += std::to_string(i);
            content += " keyword\n";
        }
        return content;
    }

    void BM_FindAndReplaceAll(benchmark::State& state)
    {
        const std::string source(static_cast<std::size_t>(state.range(0)), 'x');
        const std::string input = "token " + source + " token " + source + " token";

        for (auto _ : state)
        {
            std::string value = input;
            StringHelper::FindAndReplaceAll(value, "token", "replacement");
            benchmark::DoNotOptimize(value);
        }
    }
    BENCHMARK(BM_FindAndReplaceAll)->Arg(64)->Arg(1024);

    void BM_TrimInPlace(benchmark::State& state)
    {
        const std::string input
            = " \t\r\n" + std::string(static_cast<std::size_t>(state.range(0)), 'x') + " \t\r\n";

        for (auto _ : state)
        {
            std::string value = input;
            StringHelper::TrimInPlace(value);
            benchmark::DoNotOptimize(value);
        }
    }
    BENCHMARK(BM_TrimInPlace)->Arg(64)->Arg(1024);

    void BM_SplitString(benchmark::State& state)
    {
        std::string input;
        for (auto i = 0; i < state.range(0); ++i)
        {
            input += " field ";
            input += std::to_string(i);
            input += ',';
        }

        for (auto _ : state)
        {
            const auto tokens = StringHelper::SplitString(input);
            benchmark::DoNotOptimize(tokens);
        }
    }
    BENCHMARK(BM_SplitString)->Arg(10)->Arg(100);

    void BM_GoToLineStart(benchmark::State& state)
    {
        const std::string content = MakeLines(static_cast<std::size_t>(state.range(0)));
        const auto* begin = content.c_str();
        const auto* position = begin + content.find("line " + std::to_string(state.range(0) - 1));

        for (auto _ : state)
        {
            benchmark::DoNotOptimize(FileNavigator::GoToLineStart(position + 5, begin));
        }
    }
    BENCHMARK(BM_GoToLineStart)->Arg(100)->Arg(1'000);

    void BM_FindFirstWithLineLimit(benchmark::State& state)
    {
        const std::string content = MakeLines(static_cast<std::size_t>(state.range(0)));

        for (auto _ : state)
        {
            benchmark::DoNotOptimize(
                FileNavigator::FindFirstWithLineLimit(content.c_str(), "keyword", state.range(0)));
        }
    }
    BENCHMARK(BM_FindFirstWithLineLimit)->Arg(100)->Arg(1'000);

    void BM_GetLineNumberAndColumn(benchmark::State& state)
    {
        const std::string content = MakeLines(static_cast<std::size_t>(state.range(0)));

        for (auto _ : state)
        {
            benchmark::DoNotOptimize(
                FileNavigator::GetLineNumberAndColumn(content.c_str(), content.size() - 1));
        }
    }
    BENCHMARK(BM_GetLineNumberAndColumn)->Arg(100)->Arg(1'000);

    void BM_ReadAsTypename(benchmark::State& state)
    {
        const char* source = "static const std::vector<std::pair<int, Value>>* field";

        for (auto _ : state)
        {
            int offset = 0;
            benchmark::DoNotOptimize(FileNavigator::ReadAsTypename(source, offset));
            benchmark::DoNotOptimize(offset);
        }
    }
    BENCHMARK(BM_ReadAsTypename);

    void BM_FindScopeEnd(benchmark::State& state)
    {
        std::string source;
        source.append(static_cast<std::size_t>(state.range(0)), '{');
        source.append(static_cast<std::size_t>(state.range(0)), '}');

        for (auto _ : state)
        {
            benchmark::DoNotOptimize(FileNavigator::FindScopeEnd(source.c_str()));
        }
    }
    BENCHMARK(BM_FindScopeEnd)->Arg(10)->Arg(100);

} // namespace
