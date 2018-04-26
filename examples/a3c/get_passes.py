import re 
import os, fnmatch, sys
from pyllvm import *

def get_passes(fn='opt_passes.md'):
    passes = []
    with open(fn) as fp:
        for line in fp:
            tokens = line.split('|');
            #print (len(tokens))
            if tokens[2].strip()== "Y":
                pass_name = tokens[0]
                pass_name = pass_name.strip().replace('`', '')
                passes.append(pass_name)
                
    print(passes)
    print("Number of Passes: %d" % len(passes))
    return (passes, len(passes))

usingopts, NUM_OPTIONS = get_passes('opt_passes.md') 

# Polybench dataset options:  MINI_DATASET, SMALL_DATASET, STANDARD_DATASET, LARGE_DATASET,  EXTRALARGE_DATASET
DATASET_OPTION = '-DSMALL_DATASET' 

# Generate getPollyLLVM code
def getPollyLLVM(polyfile):
    return getLLVM(polyfile, ["-I", '../../benchmarks/polybench-c-3.2/utilities', '-include', '../../benchmarks/polybench-c-3.2/utilities/polybench.c', DATASET_OPTION])

def getRealOpts():
    opts = getOpts()
    omap = {o.getPassArgument():o for o in opts}
    opts = {a:omap[a] for a in usingopts} # if not omap[a].isAnalysis()}

    #for o in opts:
    #    if o.isAnalysis():
    #        print(o.getPassArgument(), o.getPassName())
    return opts

opts = getRealOpts()

def countPasses():
    count=len(opts)
    return count

def getTime(c_code, opt_indice):
    #opt_indice = [49,33,44,14,30,17,36]
    g = getPollyLLVM(c_code)
    success = True

    # If x is an available pass, look it up in opts, if not, return no_opt
    llvm_opts = list(map((lambda x: opts[usingopts[x]] if x < len(opts) else None), opt_indice))
    for i, o in zip(opt_indice, llvm_opts):
        if (o is not None):
            print("applying " + str(i)+"/"+str(len(opts)) + " " + o.getPassArgument() + " - " + o.getPassName())
            success = applyOpt(o, g)
            if (not success):
                print("Failed to apply opt sequence")
                print(opt_indice)
                break
        else:
            print("applying " + str(i)+"/"+str(len(opts))+" no_opt - Does nothing")
    #print(g)

    # if the pass 
    count = 10
    if (success):
        print("timing function")
        time = g.timeFunction("main", count )
        time = float(time)
        if time == float('inf'):
            time = 1000000.0 * count
    else:
        #time = float('inf')
        time = 1000000.0 * count
    print("wall time: %f"%time) 
    return time

