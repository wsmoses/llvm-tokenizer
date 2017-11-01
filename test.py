from pyllvm import *

g = getLLVM('temp.c')
print('No optimization')
g.dump()
opts = getOpts()
opts.sort(key=lambda x:x.getPassName())
#print(opts)

#for i, o in enumerate(opts):
#    print(i, o.getPassName())
print(len(opts))
for o in opts:
    print("applying " + o.getPassName())
    ran = applyOpt(o, g)
    if not ran: print("failed to run: " + o.getPassName())
    g.dump()
