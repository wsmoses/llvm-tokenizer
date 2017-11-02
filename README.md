Install pyllvm by doing
pip3 install .

Then if you import pyllvm you will (eventually) find the following functions
pyllvm:

    # Given filename, output LLVM IR Module
    def getLLVM(filename):

    # Get the list of optimization passes
    def getOpts():

    # Run individual opt
    def applyOpt(opt, ir)

    # Create Binary from IR
    def createBinary(ir)


# Dataset Infrastructure 

## CSmith
Follow the installation instructions in: [http://embed.cs.utah.edu/csmith/](http://embed.cs.utah.edu/csmith/)

The source code is [CSmith Github Page](https://github.com/csmith-project/csmith)
The template path is csmith/runtime 

## Clgen
Follow the installation instructions in: [https://chriscummins.cc/clgen/](https://chriscummins.cc/clgen/)

If run into 
:error
    could not create 'build': File exists

    move the BUILD file to BUILD.bak in the current folder and rerun `make install`.

    Run example clgen-0.3.13/clgen/test/data/tiny
    `clgen sample model.json sampler.json`

    The following sql dump does not work.
    `clgen db dump /kernels.db  ./sample`

    sqlite query
    > .tables
    ContentFiles       Meta               Repositories
    ContentMeta        PreprocessedFiles

    > pragma table_info(ContentFiles);
    0|id|TEXT|1||0
    1|contents|TEXT|1||0

