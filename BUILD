cc_library(
    name = 'octree',
    srcs = ['octree.cpp'],
    hdrs = ['octree.h'],
    compiler_flags = ['-std=c++17'],
)

cc_library(
    name = 'test_main',
    srcs = ['test_main.cpp'],
    hdrs = ['catch.hpp'],
    compiler_flags = ['-std=c++17'],
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
    compiler_flags = ['-std=c++17'],
    write_main = False
)
