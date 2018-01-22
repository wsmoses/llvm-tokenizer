from pyllvm import *

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
#for o in opts:
#    print(o.getPassArgument() + " " + o.getPassName())
#print(len(opts))

g = getLLVM('temp.c', [])
print(g)
#createBinary(g, 'temp.o')
#from sys import exit; exit(1)

#print(len(opts))

for i, k in enumerate(['mem2reg', 'licm']): #enumerate(opts):
    o = opts[k]
    print("applying " + str(i)+"/"+str(len(opts)) + " " + o.getPassArgument() + " " + o.getPassName())
    applyOpt(o, g)
    print(g)
