from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

import os
for key in sorted(os.environ.keys()):
    print('%r: %r' % (key, os.environ[key]))

os.system('ls -l')

import sys
print('args are: %s' % sys.argv)

def main():
    setup(
        ext_modules=[
            Extension(
                'octree_wrapper', 
                ['python/octree_wrapper.pyx', 'octree.cpp'], 
                extra_compile_args=['-std=c++17', '-O3'],
                language='c++',
                include_dirs=['.']
            )
        ],
        cmdclass = {'build_ext': build_ext}
    )

if __name__ == '__main__':
    main()
