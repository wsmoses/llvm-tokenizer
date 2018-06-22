from pyllvm import *

g = getLLVM('temp.c', [])
print(g)
h = g.timeFunction('main', 1)
print(h)
