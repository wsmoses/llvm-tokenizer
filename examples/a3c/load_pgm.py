import os, fnmatch, sys

def lsFiles(directory, pattern):
    print("Search C source in: %s"%directory)
    pgms = []
    for root, dirs, files in os.walk(directory):
        for basename in files:
            if fnmatch.fnmatch(basename, pattern):
                filename = os.path.join(root, basename)
                pgms.append( os.path.abspath(filename))
    return pgms

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
 
def load_pgm():

    bm_dir = "../../benchmarks/polybench-c-3.2/"
    pgms = baseline_11_polybenmarks(bm_dir)

    for pgm in pgms:
        print ('Found C source: %s'%pgm)
    return pgms
    


