# This is a Cython file and extracts the relevant classes from the C++ header file.

# cython: language_level=3
# distutils: language = c++
# distutils: sources = octree.cpp

from libcpp cimport bool
from libcpp.string cimport string

cdef extern from "<unordered_map>" namespace "std":
    cdef cppclass unordered_map[Key, Value]:
        unordered_map()

cdef extern from "<optional>" namespace "std":
    cdef cppclass optional[T]:
        bool has_value()
        T value()

ctypedef int LocationCodeType
ctypedef int NodeType

cdef extern from "octree.h":
    cdef cppclass Octree:
        Octree(bool)
        void set_root()
        void clear_root()
        void set(int)
        void clear(int)
        float get_volume()
        string to_string()

cdef class PyOctree:
    cdef Octree* _thisptr
    def __cinit__(self, bool full):
        self._thisptr = new Octree(full)
    def __dealloc__(self):
        del self._thisptr
    def set_root(self):
        return self._thisptr.set_root()
    def clear_root(self):
        return self._thisptr.clear_root()
    def set(self, LocationCodeType location_code):
        return self._thisptr.set(location_code)
    def clear(self, LocationCodeType location_code):
        return self._thisptr.clear(location_code)
    def get_volume(self):
        return self._thisptr.get_volume()
    def __str__(self):
        return self._thisptr.to_string().decode('utf-8')
