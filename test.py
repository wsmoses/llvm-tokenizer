from pyllvm import *

def getRealOpts():
    opts = getOpts()
    opts.sort(key=lambda x:x.getPassName())
    del opts[9]
    del opts[19]
    del opts[69]
    del opts[90]
    del opts[111]
    del opts[112]
    del opts[112]
    del opts[125]
    del opts[143]
    del opts[163]
    del opts[173]
    del opts[174]
    del opts[181]
    del opts[201]
    del opts[203]
    del opts[224]
    del opts[224]
    del opts[224]
    opts.sort(key=lambda x:x.getPassArgument())
    return opts

opts = getRealOpts()
#for o in opts:
#    print(o.getPassArgument() + " " + o.getPassName())

g = getLLVM('mtemp.c')
print(g)
createBinary(g, 'temp.o')
from sys import exit; exit(1)

print(len(opts))

for i, o in enumerate(opts):
    print("applying " + str(i)+"/"+str(len(opts)) + " " + o.getPassName())
    applyOpt(o, g)
    g.dump()
