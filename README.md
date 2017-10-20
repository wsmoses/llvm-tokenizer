
Backend interface

pyllvm:

    # Given filename, output LLVM IR
    def parseC(filename):

    # Get the list of optimization passes
    def getOpts():

    # Run individual opt
    def runOpt(ir, optname)

    # Create Binary from IR
    def createBinary(ir)


