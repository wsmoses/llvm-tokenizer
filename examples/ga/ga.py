from pyllvm import *
import random
import re
import subprocess
import os, fnmatch, sys
from  subprocess import call

from deap import base
from deap import creator
from deap import tools
from timeit import timeit
import time
#import os.path, sys

import get_passes
# sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), os.pardir))
# from dataset import *

# Polybench dataset options:  MINI_DATASET, SMALL_DATASET, STANDARD_DATASET, LARGE_DATASET,  EXTRALARGE_DATASET
DATASET_OPTION = '-DSMALL_DATASET' 
DATASET_OPTION = ''

# Generate getPollyLLVM code
def getPollyLLVM(polyfile):
    return getLLVM(polyfile, ["-I", '../../benchmarks/polybench-c-3.2/utilities', '-include', '../../benchmarks/polybench-c-3.2/utilities/polybench.c', DATASET_OPTION])

def lsFiles(directory, pattern):
    print("Search C source in: %s"%directory)
    pgms = []
    for root, dirs, files in os.walk(directory):
        for basename in files:
            if fnmatch.fnmatch(basename, pattern):
                filename = os.path.join(root, basename)
                pgms.append( os.path.abspath(filename))
    return pgms

# Put Y to the pass to include in opt_passes.md
usingopts, _ = get_passes.get_passes()
#alloca-hoisting dne
#amode-opt
#called-value-propagation
#div-rem-pairs
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
    if (success):
        print("timing function")
        time = g.timeFunction("main", 10)
    else:
        time = float('inf')
    print("wall time: %f"%time) 
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
    #NOTE: Add no_opt as the last optimization option
    random.seed(1)
    toolbox.register("attr_bool", random.randint, 0, countPasses())

    # Structure initializers
    #                         define 'individual' to be an individual
    #                         consisting of 100 'attr_bool' elements ('genes')
    # Number of optimization passes applied
    toolbox.register("individual", tools.initRepeat, creator.Individual,
            toolbox.attr_bool, 15)

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

# Cannot find the nussinov bm 
def baseline_11_polybenmarks(bm_dir = "../../benchmarks/polybench-c-3.2/"):
    linear_dir = 'linear-algebra/kernels/'
    linear_krnls = list(map(lambda x: linear_dir + x, ['2mm', '3mm', 'atax', 'doitgen', 'gemver', 'mvt', 'syr2k', 'syrk']))
    krnls = linear_krnls + ["datamining/correlation", "stencils/jacobi-2d-imper", "/stencils/seidel-2d"]
    krnls = list(map(lambda x: bm_dir + x, krnls))
    pgms = []
    for krnl in krnls: 
        pgms.extend(lsFiles(directory= krnl, pattern='*.c'))
    return pgms

def all_polybenmarks(bm_dir = "../../benchmarks/polybench-c-3.2/"): 
    categories = ["datamining", "linear-algebra", "medley", "stencils"]
    pgms = []
    for category in categories: 
        pgms.extend(lsFiles(directory= os.path.join(bm_dir, category) , pattern='*.c'))
    return pgms
 
 
def main(): 
    bm_dir = "../../benchmarks/polybench-c-3.2/"
    pgms = baseline_11_polybenmarks(bm_dir)

    for pgm in pgms:
        print ('Found C source: %s'%pgm)

    for pgm in pgms: 
        print ("TEST: %s"%pgm)
        # Copy to skeleton folder 
        toolbox = setupGA(pgm)
        begin = time.time()
        trainGA(toolbox)
        end = time.time()
        print("Compile Time: %d"%(int(end - begin)))
if __name__ == "__main__":
    main()
