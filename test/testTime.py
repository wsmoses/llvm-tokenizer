from pyllvm import *

g = getLLVM('temp.c', [])
print(g)
h = g.timeFunction('main')
print(h)
