#!/usr/bin/env python3
"""Exercise source-clean, incremental reflection in an isolated source archive."""

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def run(*args, success=True):
    result = subprocess.run(args, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            timeout=300, check=False)
    if (result.returncode == 0) != success:
        raise RuntimeError(f"{' '.join(map(str, args))}\n{result.stdout}")
    return result.stdout


def snapshot(root):
    return {str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in root.rglob('*') if p.is_file() and '.git' not in p.parts}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, required=True)
    args = parser.parse_args()
    build = args.build_dir.resolve()
    repo = Path(__file__).resolve().parents[1]
    output = run('cmake', '--build', str(build), '--target', 'JRMTests')
    if 'Reflecting JRMTests' in output:
        raise RuntimeError('The initial build must already be up to date.')
    stamps = list((build / 'tests').rglob('reflection.stamp'))
    if not stamps:
        raise RuntimeError('No reflection stamp found.')
    before = {str(p): p.stat().st_mtime_ns for p in stamps}
    run('cmake', '--build', str(build), '--target', 'JRMTests')
    assert before == {str(p): p.stat().st_mtime_ns for p in stamps}, 'No-op regenerated fixtures'

    with tempfile.TemporaryDirectory(prefix='jrm cmake ') as temp:
        root = Path(temp)
        source = root / 'source archive'
        source.mkdir()
        for name in ('CMakeLists.txt', 'cmake', 'sources', 'tests', 'dependencies'):
            src = repo / name
            if src.is_dir():
                shutil.copytree(src, source / name,
                                ignore=shutil.ignore_patterns('.git', '*.generated.h', 'cache.data'))
            else:
                shutil.copy2(src, source / name)
        original = snapshot(source)
        binary = root / 'build with spaces'
        run('cmake', '-S', str(source), '-B', str(binary), '-G', 'Ninja',
            '-DCMAKE_BUILD_TYPE=Debug', '-DJRM_WARNINGS_AS_ERRORS=ON',
            '-DCMAKE_CXX_COMPILER_LAUNCHER=')
        run('cmake', '--build', str(binary), '--parallel', '2')
        run('ctest', '--test-dir', str(binary), '--output-on-failure', '--no-tests=error')
        assert snapshot(source) == original, 'Build or tests changed source files'
        stamp = binary / 'tests/JRMTests-reflection/Debug/reflection.stamp'
        generated = stamp.parent / 'uat/Classes.generated.h'

        def rebuild(expected):
            old = stamp.stat().st_mtime_ns if stamp.exists() else 0
            run('cmake', '--build', str(binary), '--target', 'JRMTests', '--parallel', '2')
            assert (stamp.stat().st_mtime_ns != old) == expected, 'Unexpected regeneration'
            assert generated.stat().st_size > 0

        rebuild(False)
        generated.unlink()
        rebuild(True)
        for relative in ('tests/uat/Classes.h', 'tests/.jrm/config.yaml',
                         'sources/JustReflectMe/JustReflectMe.cpp'):
            path = source / relative
            os.utime(path, None)
            rebuild(True)
            rebuild(False)
        run('cmake', '--build', str(binary), '--target', 'clean')
        rebuild(True)
        assert snapshot(source) == original, 'Rebuild changed source content'

        for name, options, diagnostic in (
            ('no-tests-coverage', ['-DJRM_DISABLE_TESTS=ON', '-DJRM_ENABLE_CODE_COVERAGE=ON'],
             'requires JRM_DISABLE_TESTS=OFF'),
            ('html-without-coverage', ['-DJRM_GENERATE_CODE_COVERAGE_HTML=ON'],
             'requires JRM_ENABLE_CODE_COVERAGE=ON'),
            ('benchmark-coverage', ['-DJRM_ENABLE_BENCHMARKS=ON', '-DJRM_ENABLE_CODE_COVERAGE=ON'],
             'cannot both be enabled'),
            ('agents', ['-DJRM_SETUP_AGENT_SKILLS=ON'], 'pinned .agents/Agents'),
        ):
            output = run('cmake', '-S', str(source), '-B', str(root / name), *options, success=False)
            assert diagnostic in output, output

        guard = root / 'coverage-guard'
        guard.mkdir()
        report = guard / 'report'
        report.mkdir()
        (report / 'keep').write_text('preserve existing files')
        command = ['cmake', f'-DJRM_BINARY_DIR={guard}', f'-DJRM_REPORT={report}',
                   '-P', str(source / 'cmake/PrepareCoverage.cmake')]
        run(*command, success=False)
        assert (report / 'keep').read_text() == 'preserve existing files'
        if os.name != 'nt':
            outside = root / 'external'
            outside.mkdir()
            (guard / 'alias').symlink_to(outside, target_is_directory=True)
            command[2] = f'-DJRM_REPORT={guard}/alias/new'
            run(*command, success=False)
            assert not (outside / 'new').exists()

        parent = root / 'parent'
        parent.mkdir()
        (parent / 'main.cpp').write_text(
            '#include <JustReflectMe/Adapter.h>\nint main() { RJsonResourceStream stream; }\n')
        (parent / 'CMakeLists.txt').write_text(f"""cmake_minimum_required(VERSION 3.30)
project(Consumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER_LAUNCHER "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${{CMAKE_BINARY_DIR}}/parent-bin")
set(INSTALL_GTEST OFF)
set(JRM_DISABLE_TESTS OFF)
enable_testing()
add_subdirectory("{source}/dependencies/googletest" gtest)
add_subdirectory("{source}" jrm)
if(NOT CMAKE_CXX_STANDARD STREQUAL "17" OR NOT CMAKE_CXX_COMPILER_LAUNCHER STREQUAL "")
    message(FATAL_ERROR "JRM changed parent settings")
endif()
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY STREQUAL "${{CMAKE_BINARY_DIR}}/parent-bin")
    message(FATAL_ERROR "JRM changed parent output directories")
endif()
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE JustReflectMe::Adapter)
""")
        parent_build = root / 'parent-build'
        run('cmake', '-S', str(parent), '-B', str(parent_build), '-G', 'Ninja',
            '-DCMAKE_BUILD_TYPE=Debug')
        run('cmake', '--build', str(parent_build), '--target', 'consumer', '--parallel', '2')
        run(str(parent_build / 'parent-bin' / ('consumer.exe' if os.name == 'nt' else 'consumer')))

        shutil.rmtree(source / 'dependencies/googletest')
        shutil.rmtree(source / 'dependencies/benchmark')
        run('cmake', '-S', str(source), '-B', str(root / 'minimal'), '-DJRM_DISABLE_TESTS=ON')
        output = run('cmake', '-S', str(source), '-B', str(root / 'missing-tests'), success=False)
        assert 'pinned dependencies/googletest' in output, output
        output = run('cmake', '-S', str(source), '-B', str(root / 'missing-benchmark'),
                     '-DJRM_DISABLE_TESTS=ON', '-DJRM_ENABLE_BENCHMARKS=ON', success=False)
        assert 'pinned dependencies/benchmark' in output, output
        output = run('cmake', '-S', str(source), '-B', str(source), success=False)
        assert 'In-source builds are not supported' in output, output
    print('CMake regression checks passed.')


if __name__ == '__main__':
    main()
