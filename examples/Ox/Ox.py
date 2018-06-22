from pyllvm import *
import time
from multiprocessing.pool import ThreadPool
import sys, os
import numpy as np
import pickle
sys.path.append('../../utils/')
import utils
import scipy.stats

#Run Passes individually and sort the pass according to geomean
def runOx(pgm, level):
    wall_time = utils.getTimeOptLevel(pgm, level) 
    return wall_time


def main():
    bm_dir = "../../benchmarks/polybench-c-3.2/"
    pgms = utils.baseline_11_polybenmarks(bm_dir)
    pgms = pgms[0:1]

    for pgm in pgms:
        print ('Found C source: %s'%pgm)

    time_record = []
    level = 0
    dataset = utils.DATASET_OPTION

    begin = time.time()
    for pgm in pgms: 
        filename = os.path.basename(pgm).replace(".c", "")+dataset+"_O"+str(level)+".pkl"
        if os.path.exists(filename):
            record = pickle.load( open( filename, "rb" ) )
        else:
            record = runOx(pgm, level)
            
            with open(filename, "wb") as lb_file:
                pickle.dump(record, lb_file)
 
        time_record.append(record)
    print('Time: {}'.format(time_record))

    end = time.time()
    print("Compile Time: %d"%(int(end - begin)))

if __name__== "__main__":
  main()


