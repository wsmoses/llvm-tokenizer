from pyllvm import *
import random
import re
import subprocess
import os
from  subprocess import call

from deap import base
from deap import creator
from deap import tools
from timeit import timeit
import time 


import os.path, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), os.pardir))
from dataset import *

# Generate CSmith code
def generateC():
    c = Csmith(path="./deaptest/", csmith_path="", options="--no-structs --no-pointers --no-math64 --max-funcs 4 --no-unions", template_path="/usr/local/include/csmith-2.4.0/")
    c.generate(1)
    print(c.list())

def lsFiles(path="./deaptest"):
    path = os.path.abspath(path)
    p = re.compile("\d+.c")
    files = [f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f)) and p.match(f)]
    return files

usingopts = [
    'aa', 'adce', 'alignment-from-assumptions',
    'always-inline', 'argpromotion', 'assumption-cache-tracker',
    'atomic-expand', 'basicaa', 'basiccg', 'bdce',
    'block-freq', 'branch-prob', 'break-crit-edges',
    'cfinserter', 'cfl-anders-aa', 'cfl-steens-aa',
    'codegenprepare', 'consthoist', 'constmerge',
    'constprop', 'cost-model', 'cross-dso-cfi',
    'da', 'dce', 'deadargelim', 'delinearize',
    'demanded-bits', 'die', 'divergence', 'domfrontier',
    'domtree', 'dse', 'early-cse', 'early-cse-memssa',
    'elim-avail-extern', 'expand-reductions', 'external-aa',
    'flattencfg', 'float2int', 'forceattrs',
    'global-merge', 'globaldce', 'globalopt', 'globals-aa',
    'globalsplit', 'gvn', 'gvn-hoist', 'gvn-sink',
    'indvars', 'infer-address-spaces', 'inferattrs',
    'inline', 'instcombine', 'instsimplify', 'interleaved-access',
    'intervals', 'ipconstprop', 'ipsccp',
    'irce', 'iv-users', 'jump-threading', 'latesimplifycfg',
    'lcssa', 'libcalls-shrinkwrap', 'licm', 'load-store-vectorizer',
    'loop-accesses', 'loop-data-prefetch', 'loop-deletion',
    'loop-distribute', 'loop-extract', 'loop-extract-single',
    'loop-idiom', 'loop-instsimplify', 'loop-interchange',
    'loop-load-elim', 'loop-predication', 'loop-reduce', 'loop-reroll',
    'loop-rotate', 'loop-simplify', 'loop-simplifycfg', 'loop-sink',
    'loop-unroll', 'loop-unswitch', 'loop-vectorize', 'loop-versioning',
    'loop-versioning-licm', 'loops', 'lower-expect', 'lower-guard-intrinsic',
    'loweratomic', 'lowerinvoke', 'lowerswitch', 'lowertypetests',
    'machine-branch-prob', 'mem2reg', 'memcpyopt', 'memdep',
    'memoryssa', 'mergefunc', 'mergereturn', 'mldst-motion',
    'nary-reassociate', 'newgvn', 'opt-remark-emitter', 'pa-eval',
    'partial-inliner', 'partially-inline-libcalls',
    'place-backedge-safepoints-impl', 'place-safepoints',
    'postdomtree', 'pre-isel-intrinsic-lowering', 'prune-eh',
    'reassociate', 'reg2mem', 'regions', 'rewrite-statepoints-for-gc',
    'rpo-functionattrs', 'scalar-evolution', 'scalarize-masked-mem-intrin',
    'scalarizer', 'sccp', 'scev-aa', 'scoped-noalias', 'separate-const-offset-from-gep',
    'simple-loop-unswitch', 'simplifycfg', 'sink', 'slp-vectorizer', 'slsr',
    'speculative-execution', 'sroa', 'strip', 'strip-dead-debug-info',
    'strip-dead-prototypes', 'strip-debug-declare', 'strip-gc-relocates',
    'strip-nondebug', 'strip-nonlinetable-debuginfo', 'structurizecfg',
    'tailcallelim', 'targetlibinfo', 'tbaa', 'tti', 'unreachableblockelim',
    'wholeprogramdevirt'
]
# 'strip' is useful pass

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
   
    g = getLLVM(c_code)

    llvm_opts = list(map((lambda x: opts[usingopts[x]]), opt_indice))
    for i, o in zip(opt_indice, llvm_opts):
        print("applying " + str(i)+"/"+str(len(opts)) + " " + o.getPassArgument() + " " + o.getPassName())
        applyOpt(o, g)
    #print(g)

    createBinary(g, 'test')
    time = timeit(stmt = "subprocess.call('./test')", setup = "import subprocess", number = 100)
    print("wall time: ", time) 
    return time
    
def setupGA(c_code):
    # Weights=1 maximize a single objective
    creator.create("FitnessMax", base.Fitness, weights=(1.0,))
    creator.create("Individual", list, fitness=creator.FitnessMax)

    toolbox = base.Toolbox()

    # Attribute generator
    #                      define 'attr_bool' to be an attribute ('gene')
    #                      which corresponds to integers sampled uniformly
    #                      from the range [0,1] (i.e. 0 or 1 with equal
    #                      probability)
    toolbox.register("attr_bool", random.randint, 0, countPasses()-1)

    # Structure initializers
    #                         define 'individual' to be an individual
    #                         consisting of 100 'attr_bool' elements ('genes')
    # Number of optimization passes applied
    toolbox.register("individual", tools.initRepeat, creator.Individual,
        toolbox.attr_bool, 10)

    # define the population to be a list of individuals
    toolbox.register("population", tools.initRepeat, list, toolbox.individual)

    # the goal ('fitness') function to be maximized
    # Run Legup and recorde the negative cycles
    def evalOneMax(individual):
        cycle = getTime(c_code, individual)
        return -cycle,

    #----------
    # Operator registration
    #----------
    # register the goal / fitness function
    toolbox.register("evaluate", evalOneMax)

    # register the crossover operator
    toolbox.register("mate", tools.cxTwoPoint)

    # register a mutation operator with a probability to
    # flip each attribute/gene of 0.05
    toolbox.register("mutate", tools.mutFlipBit, indpb=0.1)

    # operator for selecting individuals for breeding the next
    # generation: each individual of the current generation
    # is replaced by the 'fittest' (best) of three individuals
    # drawn randomly from the current generation.
    # Pick top 3
    toolbox.register("select", tools.selTournament, tournsize=5)
    return toolbox
    #----------

def trainGA(toolbox):
    random.seed(64)

    # create an initial population of 300 individuals (where
    # each individual is a list of integers)
    pop = toolbox.population(n=300)

    # CXPB  is the probability with which two individuals
    #       are crossed
    #
    # MUTPB is the probability for mutating an individual
    #CXPB, MUTPB = 0.5, 0.2
    CXPB, MUTPB = 0.6, 0.3

    print("Start of evolution")

    # Evaluate the entire population
    fitnesses = list(map(toolbox.evaluate, pop))
    for ind, fit in zip(pop, fitnesses):
        ind.fitness.values = fit

    print("  Evaluated %i individuals" % len(pop))

    # Extracting all the fitnesses of
    fits = [ind.fitness.values[0] for ind in pop]

    # Variable keeping track of the number of generations
    g = 0

    # Begin the evolution
    # If the best gnome is larger than 0?
    while max(fits) < 0 and g < 30:
        # A new generation
        g = g + 1
        print("-- Generation %i --" % g)

        # Select the next generation individuals
        offspring = toolbox.select(pop, len(pop))
        # Clone the selected individuals
        offspring = list(map(toolbox.clone, offspring))

        # Apply crossover and mutation on the offspring
        for child1, child2 in zip(offspring[::2], offspring[1::2]):

            # cross two individuals with probability CXPB
            if random.random() < CXPB:
                toolbox.mate(child1, child2)

                # fitness values of the children
                # must be recalculated later
                del child1.fitness.values
                del child2.fitness.values

        for mutant in offspring:

            # mutate an individual with probability MUTPB
            if random.random() < MUTPB:
                toolbox.mutate(mutant)
                del mutant.fitness.values

        # Evaluate the individuals with an invalid fitness
        invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        fitnesses = map(toolbox.evaluate, invalid_ind)
        for ind, fit in zip(invalid_ind, fitnesses):
            ind.fitness.values = fit

        print("  Evaluated %i individuals" % len(invalid_ind))

        # The population is entirely replaced by the offspring
        pop[:] = offspring

        # Gather all the fitnesses in one list and print the stats
        fits = [ind.fitness.values[0] for ind in pop]

        length = len(pop)
        mean = sum(fits) / length
        sum2 = sum(x*x for x in fits)
        std = abs(sum2 / length - mean**2)**0.5

        print("  Min %s" % min(fits))
        print("  Max %s" % max(fits))
        print("  Avg %s" % mean)
        print("  Std %s" % std)

    print("-- End of (successful) evolution --")

    best_ind = tools.selBest(pop, 1)[0]
    print("Best individual is %s, %s" % (best_ind, best_ind.fitness.values))
    print("Best is %s, %s" % (getcycle.getPasses(best_ind), best_ind.fitness.values))

def main():
    generateC()
    pgms = lsFiles("./deaptest")
    print (pgms)

    for pgm in pgms: 
        # Copy to skeleton folder 

        toolbox = setupGA(pgm)

        begin = time.time()
        trainGA(toolbox)
        end = time.time()
        print("Compile Time: %d"%(int(end - begin)))

if __name__ == "__main__":
    main()
