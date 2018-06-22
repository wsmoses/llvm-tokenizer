Pass  | Description | Include? | Note
--- | --- | --- | ---
`aa` | Function Alias Analysis Results | |Analysis 
`aa-eval` | Exhaustive Alias Analysis Precision Evaluator | | 
`aarch64-a57-fp-load-balancing` | AArch64 A57 FP Load-Balancing | | 
`aarch64-ccmp` | AArch64 CCMP Pass | | 
`aarch64-collect-loh` | AArch64 Collect Linker Optimization Hint (LOH) | | 
`aarch64-condopt` | AArch64 CondOpt Pass | | 
`aarch64-copyelim` | AArch64 redundant copy elimination pass | | 
`aarch64-dead-defs` | AArch64 Dead register definitions | | 
`aarch64-expand-pseudo` | AArch64 pseudo instruction expansion pass | | 
`aarch64-fix-cortex-a53-835769-pass` | AArch64 fix for A53 erratum 835769 | | 
`aarch64-ldst-opt` | AArch64 load / store optimization pass | | 
`aarch64-local-dynamic-tls-cleanup` | AArch64 Local Dynamic TLS Access Clean-up | | 
`aarch64-promote-const` | AArch64 Promote Constant Pass | | 
`aarch64-simd-scalar` | AdvSIMD Scalar Operation Optimization | | 
`aarch64-stp-suppress` | AArch64 Store Pair Suppression | | 
`aarch64-vectorbyelement-opt` | AArch64 vector by element instruction optimization pass | | 
`adce` | Aggressive Dead Code Elimination |Y | 
`add-discriminators` | Add DWARF path discriminators | | 
`alignment-from-assumptions` | Alignment from assumptions |Y | 
`alloca-hoisting` | Hoisting alloca instructions in non-entry blocks to the entry block |N | 
`always-inline` | Inliner for always_inline functions |Y | 
`amdgpu-aa` | AMDGPU Address space based Alias Analysis | |Analysis 
`amdgpu-always-inline` | AMDGPU Inline All Functions | | 
`amdgpu-annotate-kernel-features` | Add AMDGPU function attributes | | 
`amdgpu-annotate-uniform` | Add AMDGPU uniform metadata | | 
`amdgpu-argument-reg-usage-info` | Argument Register Usage Information Storage | | 
`amdgpu-codegenprepare` | AMDGPU IR optimizations | | 
`amdgpu-inline` | AMDGPU Function Integration/Inlining | | 
`amdgpu-lower-enqueued-block` | Lower OpenCL enqueued blocks | | 
`amdgpu-lower-intrinsics` | Lower intrinsics | | 
`amdgpu-promote-alloca` | AMDGPU promote alloca to vector or LDS | | 
`amdgpu-rewrite-out-arguments` | AMDGPU Rewrite Out Arguments | | 
`amdgpu-simplifylib` | Simplify well-known AMD library calls | | 
`amdgpu-unify-divergent-exit-nodes` | Unify divergent function exit nodes | | 
`amdgpu-unify-metadata` | Unify multiple OpenCL metadata due to linking | | 
`amdgpu-usenative` | Replace builtin math calls with that native versions. | | 
`amode-opt` | Optimize addressing mode |N | How?
`argpromotion` | Promote 'by reference' arguments to scalars |Y | 
`arm-cp-islands` | ARM constant island placement and branch shortening pass | | 
`arm-execution-deps-fix` | ARM Execution Dependency Fix | | 
`arm-ldst-opt` | ARM load / store optimization pass | | 
`arm-prera-ldst-opt` | ARM pre- register allocation load / store optimization pass | | 
`arm-pseudo` | ARM pseudo instruction expansion pass | | 
`asan` | AddressSanitizer: detects use-after-free and out-of-bounds bugs. | |Debug 
`asan-module` | AddressSanitizer: detects use-after-free and out-of-bounds bugs.ModulePass | |Debug 
`assumption-cache-tracker` | Assumption Cache Tracker | | 
`atomic-expand` | Expand Atomic instructions | | 
`barrier` | A No-Op Barrier Pass | | 
`basicaa` | Basic Alias Analysis (stateless AA impl) | |Analysis 
`basiccg` | CallGraph Construction | |Analysis 
`bdce` | Bit-Tracking Dead Code Elimination |Y | 
`block-freq` | Block Frequency Analysis | |Analysis 
`bool-ret-to-int` | Convert i1 constants to i32/i64 if they are returned | | 
`bounds-checking` | Run-time bounds checking | | 
`branch-prob` | Branch Probability Analysis | |Analysis 
`break-crit-edges` | Break critical edges in CFG | |Break all of the critical edges in the CFG by inserting a dummy basic block 
`called-value-propagation` | Called Value Propagation |N | 
`callsite-splitting` | Call-site splitting | | 
`cfl-anders-aa` | Inclusion-Based CFL Alias Analysis | |Analysis 
`cfl-steens-aa` | Unification-Based CFL Alias Analysis | |Analysis 
`codegenprepare` | Optimize for code generation |Y |What opt? 
`consthoist` | Constant Hoisting |Y | 
`constmerge` | Merge Duplicate Global Constants |Y | 
`constprop` | Simple constant propagation |Y | 
`coro-cleanup` | Lower all coroutine related intrinsics | |Coroutine 
`coro-early` | Lower early coroutine intrinsics | |Coroutine 
`coro-elide` | Coroutine frame allocation elision and indirect calls replacement | |Coroutine 
`coro-split` | Split coroutine into a set of functions driving its state machine | |Coroutine 
`correlated-propagation` | Value Propagation |Y | 
`cost-model` | Cost Model Analysis | |Analysis 
`cross-dso-cfi` | Cross-DSO CFI | |? 
`da` | Dependence Analysis | |Analysis 
`dce` | Dead Code Elimination |Y | 
`deadargelim` | Dead Argument Elimination |Y | 
`deadarghaX0r` | Dead Argument Hacking (BUGPOINT USE ONLY; DO NOT USE) | | 
`delinearize` | Delinearization |Y? |? 
`demanded-bits` | Demanded bits analysis | |Analysis 
`dfsan` | DataFlowSanitizer: dynamic data flow analysis. | |Analysis 
`die` | Dead Instruction Elimination |Y | 
`div-rem-pairs` | Hoist/decompose integer division and remainder |N | 
`divergence` | Divergence Analysis | |Analysis 
`domfrontier` | Dominance Frontier Construction | |Analysis 
`domtree` | Dominator Tree Construction | |Analysis 
`dot-callgraph` | Print call graph to 'dot' file | | 
`dot-cfg` | Print CFG of function to 'dot' file | | 
`dot-cfg-only` | Print CFG of function to 'dot' file (with no function bodies) | | 
`dot-dom` | Print dominance tree of function to 'dot' file | | 
`dot-dom-only` | Print dominance tree of function to 'dot' file (with no function bodies) | | 
`dot-postdom` | Print postdominance tree of function to 'dot' file | | 
`dot-postdom-only` | Print postdominance tree of function to 'dot' file (with no function bodies) | | 
`dot-regions` | Print regions of function to 'dot' file | | 
`dot-regions-only` | Print regions of function to 'dot' file (with no function bodies) | | 
`dse` | Dead Store Elimination |Y | 
`dwarfehprepare` | Prepare DWARF exceptions | | 
`early-cse` | Early CSE |Y | 
`early-cse-memssa` | Early CSE w/ MemorySSA |Y | 
`ee-instrument` | Instrument function entry/exit with calls to e.g. mcount() (pre inlining) | | 
`elim-avail-extern` | Eliminate Available Externally Globals |Y | 
`esan` | EfficiencySanitizer: finds performance issues. | | 
`expand-reductions` | Expand reduction intrinsics | | 
`expandmemcmp` | Expand memcmp() to load/stores | |? 
`external-aa` | External Alias Analysis | |Analysis 
`extract-blocks` | Extract Basic Blocks From Module (for bugpoint use) | | 
`falkor-hwpf-fix` | Falkor HW Prefetch Fix | | 
`falkor-hwpf-fix-late` | Falkor HW Prefetch Fix Late Phase | | 
`flattencfg` | Flatten the CFG | |? 
`float2int` | Float to int | |? 
`forceattrs` | Force set function attributes | | 
`function-import` | Summary Based Function Import | | 
`functionattrs` | Deduce function attributes | | 
`generic-to-nvvm` | Ensure that the global variables are in the global address space | | 
`global-merge` | Merge global variables |Y | 
`globaldce` | Dead Global Elimination |Y | 
`globalopt` | Global Variable Optimizer |Y | 
`globals-aa` | Globals Alias Analysis | |Analysis 
`globalsplit` | Global splitter | |?
`guard-widening` | Widen guards | | 
`gvn` | Global Value Numbering | | 
`gvn-hoist` | Early GVN Hoisting of Expressions | | 
`gvn-sink` | Early GVN sinking of Expressions | | 
`hexagon-cext-opt` | Hexagon constant-extender optimization |? | Global Instr Scheduling
`hexagon-early-if` | Hexagon early if conversion |? | 
`hexagon-gen-mux` | Hexagon generate mux instructions |? | 
`hexagon-loop-idiom` | Recognize Hexagon-specific loop idioms | | 
`hexagon-nvj` | Hexagon NewValueJump | | 
`hexagon-packetizer` | Hexagon Packetizer | | 
`hexagon-rdf-opt` | Hexagon RDF optimizations | | 
`hexagon-vlcr` | Hexagon-specific predictive commoning for HVX vectors | | 
`hwloops` | Hexagon Hardware Loops | | 
`indvars` | Induction Variable Simplification |Y | 
`infer-address-spaces` | Infer address spaces |Y | 
`inferattrs` | Infer set function attributes | | 
`inline` | Function Integration/Inlining |Y | 
`insert-gcov-profiling` | Insert instrumentation for GCOV profiling | | 
`instcombine` | Combine redundant instructions |Y | 
`instcount` | Counts the various types of Instructions | | 
`instnamer` | Assign names to anonymous instructions | | 
`instrprof` | Frontend instrumentation-based coverage lowering. | | 
`instruction-select` | Select target instructions out of generic instructions | | 
`instsimplify` | Remove redundant instructions |Y | 
`interleaved-access` | Lower interleaved memory accesses to target specific intrinsics |Y | 
`internalize` | Internalize Global Symbols |Y | 
`intervals` | Interval Partition Construction | | 
`ipconstprop` | Interprocedural constant propagation |Y | 
`ipsccp` | Interprocedural Sparse Conditional Constant Propagation |Y | 
`irce` | Inductive range check elimination | | 
`irtranslator` | IRTranslator LLVM IR -> MI | | 
`isel` | AMDGPU DAG->DAG Pattern Instruction Selection | | 
`iv-users` | Induction Variable Users | | 
`jump-threading` | Jump Threading | | 
`lazy-block-freq` | Lazy Block Frequency Analysis | |Analysis 
`lazy-branch-prob` | Lazy Branch Probability Analysis | |Analysis 
`lazy-value-info` | Lazy Value Information Analysis | |Analysis 
`lcssa` | Loop-Closed SSA Form Pass | | 
`lcssa-verification` | LCSSA Verifier | | 
`legalizer` | Legalize the Machine IR a function's Machine IR | | 
`libcalls-shrinkwrap` | Conditionally eliminate dead library calls |Y | 
`licm` | Loop Invariant Code Motion |Y | 
`lint` | Statically lint-checks LLVM IR | | 
`liveintervals` | Live Interval Analysis | |Analysis 
`load-store-vectorizer` | Vectorize load and store instructions | | 
`localizer` | Move/duplicate certain instructions close to their use | | 
`loop-accesses` | Loop Access Analysis | |Analysis 
`loop-data-prefetch` | Loop Data Prefetch |Y | 
`loop-deletion` | Delete dead loops |Y | 
`loop-distribute` | Loop Distribution | |? 
`loop-extract` | Extract loops into new functions |Y | 
`loop-extract-single` | Extract at most one loop into a new function |Y | 
`loop-idiom` | Recognize loop idioms | |?
`loop-instsimplify` | Simplify instructions in loops |Y | 
`loop-interchange` | Interchanges loops for cache reuse |Y | 
`loop-load-elim` | Loop Load Elimination |Y | 
`loop-predication` | Loop predication |Y | 
`loop-reduce` | Loop Strength Reduction |Y | 
`loop-reroll` | Reroll loops |Y | 
`loop-rotate` | Rotate Loops |Y | 
`loop-simplify` | Canonicalize natural loops |Y | 
`loop-simplifycfg` | Simplify loop CFG |Y | 
`loop-sink` | Loop Sink |Y | 
`loop-unroll` | Unroll loops |Y | 
`loop-unswitch` | Unswitch loops |Y | 
`loop-vectorize` | Loop Vectorization |Y | 
`loop-versioning` | Loop Versioning | |? 
`loop-versioning-licm` | Loop Versioning For LICM |Y | 
`loops` | Natural Loop Information | | 
`lower-expect` | Lower 'expect' Intrinsics | | 
`lower-guard-intrinsic` | Lower the guard intrinsic to normal control flow |? | 
`loweratomic` | Lower atomic intrinsics to non-atomic form |? | 
`lowerinvoke` | Lower invoke and unwind, for unwindless code generators |? | 
`lowerswitch` | Lower SwitchInst's to branches | | 
`lowertypetests` | Lower type metadata | | 
`machine-block-freq` | Machine Block Frequency Analysis | |Analysis 
`machine-branch-prob` | Machine Branch Probability Analysis | |Analysis 
`machine-domfrontier` | Machine Dominance Frontier Construction | | 
`machine-loops` | Machine Natural Loop Construction | | 
`machine-trace-metrics` | Machine Trace Metrics | | 
`machinedomtree` | MachineDominator Tree Construction | | 
`mem2reg` | Promote Memory to Register |Y | Important but would cause crash
`memcpyopt` | MemCpy Optimization |Y | 
`memdep` | Memory Dependence Analysis | | 
`memoryssa` | Memory SSA | | 
`mergefunc` | Merge Functions | | 
`mergeicmps` | Merge contiguous icmps into a memcmp |N | 
`mergereturn` | Unify function exit nodes | | 
`metarenamer` | Assign new names to everything | | 
`mldst-motion` | MergedLoadStoreMotion | | 
`module-debuginfo` | Decodes module-level debug info | | 
`module-summary-analysis` | Module Summary Analysis | | 
`msan` | MemorySanitizer: detects uninitialized reads. | | 
`name-anon-globals` | Provide a name to nameless globals | | 
`nary-reassociate` | Nary reassociation | | 
`newgvn` | Global Value Numbering | | 
`nvptx-assign-valid-global-names` | Assign valid PTX names to globals | | 
`nvptx-lower-aggr-copies` | Lower aggregate copies, and llvm.mem* intrinsics into loops | | 
`nvptx-lower-alloca` | Lower Alloca | | 
`nvptx-lower-args` | Lower arguments (NVPTX) | | 
`nvvm-intr-range` | Add !range metadata to NVVM intrinsics. | | 
`nvvm-reflect` | Replace occurrences of nvvm_reflect() calls with 0/1 | | 
`objc-arc` | ObjC ARC optimization | | 
`objc-arc-aa` | ObjC-ARC-Based Alias Analysis | | 
`objc-arc-apelim` | ObjC ARC autorelease pool elimination | | 
`objc-arc-contract` | ObjC ARC contraction | | 
`objc-arc-expand` | ObjC ARC expansion | | 
`opt-remark-emitter` | Optimization Remark Emitter | | 
`pa-eval` | Evaluate ProvenanceAnalysis on all pairs | | 
`packets` | R600 Packetizer | | 
`partial-inliner` | Partial Inliner | | 
`partially-inline-libcalls` | Partially inline calls to library functions | | 
`pgo-icall-prom` | Use PGO instrumentation profile to promote indirect calls to direct calls. | | 
`pgo-instr-gen` | PGO instrumentation. | | 
`pgo-instr-use` | Read PGO instrumentation profile. | | 
`pgo-memop-opt` | Optimize memory intrinsic using its size value profile | | 
`place-backedge-safepoints-impl` | Place Backedge Safepoints | | 
`place-safepoints` | Place Safepoints | | 
`post-inline-ee-instrument` | Instrument function entry/exit with calls to e.g. mcount() (post inlining) | | 
`postdomtree` | Post-Dominator Tree Construction | | 
`ppc-expand-isel` | PowerPC Expand ISEL Generation | | 
`ppc-tls-dynamic-call` | PowerPC TLS Dynamic Call Fixup | | 
`pre-isel-intrinsic-lowering` | Pre-ISel Intrinsic Lowering | | 
`print-alias-sets` | Alias Set Printer | | 
`print-bb` | Print BB to stderr | | 
`print-callgraph` | Print a call graph | | 
`print-callgraph-sccs` | Print SCCs of the Call Graph | | 
`print-cfg-sccs` | Print SCCs of each function CFG | | 
`print-dom-info` | Dominator Info Printer | | 
`print-externalfnconstants` | Print external fn callsites passed constants | | 
`print-function` | Print function to stderr | | 
`print-lazy-value-info` | Lazy Value Info Printer Pass | | 
`print-memdeps` | Print MemDeps of function | | 
`print-memderefs` | Memory Dereferenciblity of pointers in function | | 
`print-memoryssa` | Memory SSA Printer | | 
`print-module` | Print module to stderr | | 
`print-predicateinfo` | PredicateInfo Printer | | 
`profile-summary-info` | Profile summary info | | 
`prune-eh` | Remove unused exception handling info | | 
`r600-expand-special-instrs` | R600ExpandSpecialInstrs | | 
`r600cf` | R600 Control Flow Finalizer | | 
`r600mergeclause` | R600 Clause Merge | | 
`reassociate` | Reassociate expressions |Y | 
`reg2mem` | Demote all values to stack slots |Y | 
`regbankselect` | Assign register bank of generic virtual registers | | 
`regions` | Detect single entry single exit regions | | 
`rewrite-statepoints-for-gc` | Make relocations explicit at statepoints | | 
`rewrite-symbols` | Rewrite Symbols | | 
`rpo-functionattrs` | Deduce function attributes in RPO | | 
`safe-stack` | Safe Stack instrumentation pass | | 
`sample-profile` | Sample Profile loader | | 
`sancov` | SanitizerCoverage: TODO.ModulePass | | 
`scalar-evolution` | Scalar Evolution Analysis | | 
`scalarize-masked-mem-intrin` | Scalarize unsupported masked memory intrinsics |Y | 
`scalarizer` | Scalarize vector operations |Y | 
`sccp` | Sparse Conditional Constant Propagation |Y | 
`scev-aa` | ScalarEvolution-based Alias Analysis | |Analysis 
`scoped-noalias` | Scoped NoAlias Alias Analysis | |Analysis 
`separate-const-offset-from-gep` | Split GEPs to a variadic base and a constant offset for better CSE |Y | 
`si-annotate-control-flow` | Annotate SI Control Flow | | 
`si-debugger-insert-nops` | SI Debugger Insert Nops | | 
`si-fix-sgpr-copies` | SI Fix SGPR copies | | 
`si-fix-vgpr-copies` | SI Fix VGPR copies | | 
`si-fix-wwm-liveness` | SI fix WWM liveness | | 
`si-fold-operands` | SI Fold Operands | | 
`si-i1-copies` | SI Lower i1 Copies | | 
`si-insert-skips` | SI insert s_cbranch_execz instructions | | 
`si-insert-waitcnts` | SI Insert Waitcnts | | 
`si-insert-waits` | SI Insert Waits | | 
`si-load-store-opt` | SI Load / Store Optimizer | | 
`si-lower-control-flow` | SI lower control flow | | 
`si-memory-legalizer` | SI Memory Legalizer | | 
`si-optimize-exec-masking` | SI optimize exec mask operations | | 
`si-optimize-exec-masking-pre-ra` | SI optimize exec mask operations pre-RA | | 
`si-peephole-sdwa` | SI Peephole SDWA | | 
`si-shrink-instructions` | SI Shrink Instructions | | 
`si-wqm` | SI Whole Quad Mode | | 
`simple-loop-unswitch` | Simple unswitch loops |Y | 
`simplifycfg` | Simplify the CFG |Y | 
`sink` | Code sinking |Y | 
`sjljehprepare` | Prepare SjLj exceptions | | 
`slotindexes` | Slot index numbering | | 
`slp-vectorizer` | SLP Vectorizer |Y | 
`slsr` | Straight line strength reduction | | 
`speculative-execution` | Speculatively execute instructions |? |Seems very aggressive, would cause crash 
`sroa` | Scalar Replacement Of Aggregates |Y | 
`strip` | Strip all symbols from a module | | 
`strip-dead-debug-info` | Strip debug info for unused symbols | | 
`strip-dead-prototypes` | Strip Unused Function Prototypes | | 
`strip-debug-declare` | Strip all llvm.dbg.declare intrinsics | | 
`strip-gc-relocates` | Strip gc.relocates inserted through RewriteStatepointsForGC | | 
`strip-nondebug` | Strip all symbols, except dbg symbols, from a module | | 
`strip-nonlinetable-debuginfo` | Strip all debug info except linetables | | 
`structurizecfg` | Structurize the CFG | | 
`tailcallelim` | Tail Call Elimination | | 
`targetlibinfo` | Target Library Information | | 
`targetpassconfig` | Target Pass Configuration | | 
`tbaa` | Type-Based Alias Analysis | | 
`tsan` | ThreadSanitizer: detects data races. | | 
`tti` | Target Transform Information | | 
`unreachableblockelim` | Remove unreachable blocks from the CFG | | 
`vec-merger` | R600 Vector Reg Merger | | 
`verify` | Module Verifier | | 
`verify-safepoint-ir` | Safepoint IR Verifier | | 
`view-callgraph` | View call graph | | 
`view-cfg` | View CFG of function | | 
`view-cfg-only` | View CFG of function (with no function bodies) | | 
`view-dom` | View dominance tree of function | | 
`view-dom-only` | View dominance tree of function (with no function bodies) | | 
`view-postdom` | View postdominance tree of function | | 
`view-postdom-only` | View postdominance tree of function (with no function bodies) | | 
`view-regions` | View regions of function | | 
`view-regions-only` | View regions of function (with no function bodies) | | 
`wholeprogramdevirt` | Whole program devirtualization | | 
`winehprepare` | Prepare Windows exceptions | | 
`write-bitcode` | Write Bitcode | | 
`x86-cf-opt` | X86 Call Frame Optimization | | 
`x86-cmov-conversion` | X86 cmov Conversion | | 
`x86-domain-reassignment` | X86 Domain Reassignment Pass | | 
`x86-evex-to-vex-compress` | Compressing EVEX instrs to VEX encoding when possible | | 
`x86-execution-deps-fix` | X86 Execution Dependency Fix | | 
`x86-fixup-LEAs` | X86 LEA Fixup | | 
`x86-fixup-bw-insts` | X86 Byte/Word Instruction Fixup | | 
`x86-winehstate` | Insert stores for EH state numbers | | 
