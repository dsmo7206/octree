cc_library(
    name = 'octree',
    srcs = ['octree.cpp'],
    hdrs = ['octree.h'],
    compiler_flags = ['-std=c++17', '-O3'],
)

cc_library(
    name = 'test_main',
    srcs = ['test_main.cpp'],
    hdrs = ['catch.hpp'],
    compiler_flags = ['-std=c++17', '-O3'],
)

filegroup(
    name = 'sources',
    srcs = ['octree.cpp', 'octree.h'],
    visibility = ['PUBLIC'],
)

cc_test(
    name = 'octree_test',
    srcs = ['octree_test.cpp'],
    hdrs = ['catch.hpp'],
    deps = [':octree', ':test_main'],
    compiler_flags = ['-std=c++17', '-O3', '-mbmi2'],
    linker_flags = ['-O3'],
    write_main = False
)
