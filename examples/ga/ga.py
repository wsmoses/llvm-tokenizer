from pyllvm import *
import random
import re
import subprocess
import os, fnmatch, sys
from  subprocess import call
import numpy

from deap import base
from deap import creator
from deap import tools
from timeit import timeit
import time
#import os.path, sys
import plot_ga

import get_passes
import pickle

# sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), os.pardir))
# from dataset import *

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
    toolbox.register("attr_bool", random.randint, 0, get_passes.NUM_OPTIONS)

    # Structure initializers
    #                         define 'individual' to be an individual
    #                         consisting of 100 'attr_bool' elements ('genes')
    # Number of optimization passes applied
    toolbox.register("individual", tools.initRepeat, creator.Individual,
            toolbox.attr_bool, 60)

    # define the population to be a list of individuals
    toolbox.register("population", tools.initRepeat, list, toolbox.individual)

    # the goal ('fitness') function to be maximized
    # Run Legup and recorde the negative cycles
    def evalOneMax(individual):
        cycle = get_passes.getTime(c_code, individual)
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
    toolbox.register("select", tools.selTournament, tournsize=3)
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

    # Variable keeping track of tDSMALL_DATASEThe number of generations
    g = 0

    # Begin the evolution
    # If the best gnome is larger than 0?
    while max(fits) < 0 and g < 10:
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

        # Gather Stats 
        stats = tools.Statistics(key=lambda ind: ind.fitness.values)
        stats.register("avg", numpy.mean)
        stats.register("std", numpy.std)
        stats.register("min", numpy.min)
        stats.register("max", numpy.max)
        record = stats.compile(pop)
        print(record)   
        
        # Generate logbook
        logbook.record(gen=g, evals=max(fits), **record)
        #gen, avg, min, max = logbook.select("gen", "avg", "min", "max")
        logbook.header = "gen", "avg", "min", "max"
        print(logbook)

    print("-- End of (successful) evolution --")

    best_ind = tools.selBest(pop, 1)[0]
    print("Best individual is %s, %s" % (best_ind, best_ind.fitness.values))
    #print("Best is %s, %s" % (get_passes.getPasses(best_ind), best_ind.fitness.values))


 
def main(): 
    bm_dir = "../../benchmarks/polybench-c-3.2/"
    pgms = get_passes.baseline_11_polybenmarks(bm_dir)
    #pgms = pgms[3:]

    for pgm in pgms:
        print ('Found C source: %s'%pgm)

    for pgm in pgms: 

        global logbook 
        logbook = tools.Logbook()
        print ("TEST: %s"%pgm)
        # Copy to skeleton folder 
        toolbox = setupGA(pgm)
        begin = time.time()
        trainGA(toolbox)
        end = time.time()
        print("Compile Time: %d"%(int(end - begin)))
        
        # Save logbook for each     
        filename = os.path.basename(pgm).replace(".c", "") +"_logbook.pkl"
        if os.path.exists(filename):
            os.remove(filename)
        with open(filename, "wb") as lb_file:
            pickle.dump(logbook, lb_file)
        #plot_ga.plot_ga(logbook)
if __name__ == "__main__":
    main()
