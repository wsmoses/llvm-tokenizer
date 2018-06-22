from pyllvm import *
import time
from multiprocessing.pool import ThreadPool
import sys, os
import pickle
import numpy as np

sys.path.append('../../utils/')
import utils

# Take top N tested passes to run the insertion for the new pass 
def runInsertionN(pgm, N):

    pool = ThreadPool(processes=N)

    top_passes = [[] for idx in range(N)]
    top_timings = [10000000.0 for idx in range(N)]

    i = 0 # Walk through the pass pool 3 times 
    j = 0 # If the top performance is not changing, just iterate thru the pass pool 1 more time
    # Foreach pass we insert it to existing passes 
    while (i < utils.NUM_OPTIONS * 3 or j < utils.NUM_OPTIONS): 
        print('Top passes: {}'.format(top_passes))
        async_result = []
        pass_record = []
        time_record = []
        cur_pass = i % utils.NUM_OPTIONS
        for j in range(N):
            (passes, wall_time) = runInsertion(pgm, top_passes[j], cur_pass)
            pass_record.extend(passes)
            time_record.extend(wall_time)
    
       # Multi processes implementation
       # for j in range(N):
       #     async_result.append(pool.apply_async(runInsertion, (pgm, top_passes[j], cur_pass)))
       # for j in range(N):
       #     (passes, wall_time) = async_result[j].get()
       #     # Get current passes and the top passes from prev iter
       #     pass_record.extend(passes)
       #     time_record.extend(wall_time)

        pass_record.extend(top_passes)
        time_record.extend(top_timings)
        print(time_record)
        np_time_record = np.array(time_record)
        indices = np_time_record.argsort().flatten()[:N]
        print(indices)
        top_passes = [pass_record[idx] for idx in indices]
        new_top_timings = [time_record[idx] for idx in indices]
        
        improve = False 
        for idx in range(len(new_top_timings)):
            if (new_top_timings[idx] < top_timings[idx]):
                improve = True      
       
            top_timings[idx] = new_top_timings[idx]
        if not improve:
            j += 1
        i+=1
        if i % utils.NUM_OPTIONS == 0:
            walk = i / utils.NUM_OPTIONS
            print("Round %d start"%walk)
    return (top_passes, top_timings)

def runInsertion(pgm, cur_passes, new_pass):
    pass_record = []
    time_record = []
    for j in range(len(cur_passes)+ 1):
        test_passes = np.insert(cur_passes, j, new_pass).astype(int).tolist()
        print('Test passes: {}.'.format(test_passes))
        wall_time = utils.getTime(pgm, test_passes) 
        pass_record.append(test_passes)
        time_record.append(wall_time)
    return (pass_record, time_record)
            
def main():

    bm_dir = "../../benchmarks/polybench-c-3.2/"
    pgms = utils.baseline_11_polybenmarks(bm_dir)
    begin = time.time()
    dataset = utils.DATASET_OPTION

    for pgm in pgms:
        print ('Found C source: %s'%pgm)
    top_passes = []
    top_timings = []
    for pgm in pgms: 
        filename = os.path.basename(pgm).replace(".c", "") + dataset + "_record.pkl"
        if os.path.exists(filename):
            (passes, timings)= pickle.load( open( filename, "rb" ) )
        else:
            (passes, timings) = runInsertionN(pgm, 3)
            record = (passes, timings)
            with open(filename, "wb") as lb_file:
                pickle.dump(record, lb_file)
        top_passes.append(passes)
        top_timings.append(timings)
    print(top_passes)
    print(top_timings)
    end = time.time()
    print("Compile Time: %d"%(int(end - begin)))

if __name__== "__main__":
  main()


