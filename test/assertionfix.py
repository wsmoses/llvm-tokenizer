from pyllvm import *

opts = getOpts()
omap = {o.getPassArgument():o for o in opts}

fails = [35, 34, 33, 32, 31, 25, 24, 8, 7, 36, 28, 32, 42, 43, 46, 47, 45, 49, 44, 39, 41, 40, 38, 37, 34, 35, 52, 31, 30, 33, 5, 29, 27, 6, 5, 4, 3, 2, 1, 12, 13, 15, 17, 23, 19, 21, 22, 20, 18, 16, 14, 30, 11, 10, 50, 51, 53, 55, 54, 61, 59, 57, 58, 36, 56, 0, 1, 23, 2, 3, 8, 4, 6, 7, 10, 12, 18, 14, 15, 17, 11, 22, 16, 20, 24, 25, 28, 27, 19, 29, 13, 21, 9, 60, 0, 48]

polydir="../benchmarks/polybench-c-3.2/"
args = ["-I", polydir+"utilities", "-include", polydir+"utilities/polybench.c", "-DSMALL_DATASET"]
g = getLLVM(polydir+"linear-algebra/kernels/gemver/gemver.c", args)

for o in fails:
    print(opts[o].getPassArgument())
    foo = applyOpt(opts[o], g)
    print(foo)

#print(g)