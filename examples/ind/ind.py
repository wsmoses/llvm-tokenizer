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
def runPasses(pgm):
    N = utils.NUM_OPTIONS
    pool = ThreadPool(processes=1)

    async_result = []
    time_record = []

    for i in range(N):
        wall_time = runPass(pgm, i)
        time_record.append(wall_time)
   # for i in range(N):
   #     async_result.append(pool.apply_async(runPass, (pgm, i)))
   # for i in range(N):
   #     wall_time = async_result[i].get()
   #     time_record.append(wall_time)
    return time_record

def runPass(pgm, test_pass):
    wall_time = utils.getTime(pgm, [test_pass]) 
    return wall_time

def getBestPass(record):
    return list(map(lambda x: np.argmin(x), record))

def getGeoMean(record):
    return scipy.stats.mstats.gmean(record, axis=0)

def getMean(record):
    return np.mean(record, axis=0)
 
def main():
    bm_dir = "../../benchmarks/polybench-c-3.2/"
    pgms = utils.baseline_11_polybenmarks(bm_dir)
    dataset = utils.DATASET_OPTION

    for pgm in pgms:
        print ('Found C source: %s'%pgm)

    time_record = []

    begin = time.time()
    for pgm in pgms: 

        filename = os.path.basename(pgm).replace(".c", "")+dataset+"_record.pkl"
        if os.path.exists(filename):
            record = pickle.load( open( filename, "rb" ) )
        else:
            record = runPasses(pgm)
            
            with open(filename, "wb") as lb_file:
                pickle.dump(record, lb_file)
 
        time_record.append(record)
        print('Time: {}'.format(time_record))

    # The best pass for each pgm   
    bestpass = getBestPass(time_record)

    print(bestpass)
    list(map(lambda x: print(x), pgms))
    list(map(lambda x: utils.passToStr(x), bestpass))

    #
    np_time_record = np.array(time_record)
    
    print(getGeoMean(np_time_record))
    print(getMean(np_time_record))
    
    end = time.time()
    print("Compile Time: %d"%(int(end - begin)))

if __name__== "__main__":
  main()


