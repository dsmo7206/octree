cc_library(
    name = 'octree',
    srcs = [],
    hdrs = ['octree.h', 'octree.inl.h', 'location_code.h', 'location_code.inl.h'],
    deps = [],
    compiler_flags = ['-std=c++17', '-O3', '-mbmi2'],
)
'''
cc_library(
    name = 'hopscotch',
    srcs = [],
    hdrs = [
        'bhopscotch_map.h',
        'bhopscotch_set.h',
        'hopscotch_growth_policy.h',
        'hopscotch_hash.h',
        'hopscotch_map.h',
        'hopscotch_set.h',
    ],
    compiler_flags = ['-std=c++17', '-O3', '-mbmi2'],
)'''

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
    flags = '-r junit',
    compiler_flags = ['-std=c++17', '-O3', '-mbmi2'],
    linker_flags = ['-O3'],
    write_main = False
)
