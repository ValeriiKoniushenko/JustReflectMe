#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

namespace TestSupport
{
    namespace fs = std::filesystem;

    class TemporaryDirectory final
    {
    public:
        TemporaryDirectory()
        {
            static std::size_t sequence = 0;
            const auto unique
                = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
            _path = fs::temp_directory_path()
                    / ("just_reflect_me_tests_" + unique + "_" + std::to_string(sequence++));
            fs::create_directories(_path);
        }

        TemporaryDirectory(const TemporaryDirectory&) = delete;
        TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

        TemporaryDirectory(TemporaryDirectory&& other) noexcept
            : _path(std::exchange(other._path, {}))
        {
        }

        TemporaryDirectory& operator=(TemporaryDirectory&& other) noexcept
        {
            if (this != &other)
            {
                cleanup();
                _path = std::exchange(other._path, {});
            }
            return *this;
        }

        ~TemporaryDirectory() { cleanup(); }

        [[nodiscard]] const fs::path& path() const noexcept { return _path; }

        [[nodiscard]] fs::path writeFile(const fs::path& relativePath,
                                         const std::string& content) const
        {
            const auto target = _path / relativePath;
            fs::create_directories(target.parent_path());
            std::ofstream stream(target, std::ios::binary);
            stream << content;
            return target;
        }

        [[nodiscard]] std::string readFile(const fs::path& relativePath) const
        {
            std::ifstream stream(_path / relativePath, std::ios::binary);
            return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
        }

    private:
        void cleanup() noexcept
        {
            std::error_code error;
            fs::remove_all(_path, error);
        }
        fs::path _path;
    };
} // namespace TestSupport
