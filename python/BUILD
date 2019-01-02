filegroup(
    name = 'wrapper_files',
    srcs = [
        'setup.py', 
        'octree_wrapper.pyx',
    ],
)

genrule(
    name = 'octree_wrapper',
    cmd = 'python3 $(location setup.py) build_ext --inplace',
    deps = [
        ':wrapper_files', 
        '//:sources',
        '//third_party/python:Cython',
    ],
    outs = ['octree_wrapper.cpython-36m-x86_64-linux-gnu.so']
)

python_binary(
    name = 'octree_test',
    main = 'octree_test.py',
    deps = [':octree_wrapper'],
)