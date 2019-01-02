from python import octree_wrapper

o = octree_wrapper.PyOctree(False)
o.set(int('1000', 2))
o.set(int('1001010', 2))
print(o)
