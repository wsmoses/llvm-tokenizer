#include "stats.h"

#include <iostream>
#include <memory>
#include <string>
#include <cstring>

#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>


#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Tool.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/PCHContainerOperations.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Sema/Sema.h>

#include <llvm/PassInfo.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Option/ArgList.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace clang;

using namespace clang;
using namespace clang::driver;

std::shared_ptr<llvm::Module> getLLVM(const std::string filename, const std::vector<std::string> addl_args) {

    llvm::LLVMContext* context = new llvm::LLVMContext();

    {
    EmitLLVMOnlyAction Act(context);

    std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);
    auto opts = new DiagnosticOptions;
    TextDiagnosticPrinter printer(llvm::errs(), opts);
    std::unique_ptr<DiagnosticsEngine> Diags(new DiagnosticsEngine(IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs), opts, &printer, false));
    Driver TheDriver(CLANG_BINARY, llvm::sys::getProcessTriple(), *Diags.get());

    SmallVector<const char *, 2> Args {"clang", filename.c_str()};

    for(const std::string s: addl_args) {
        char* ns = new char[s.length()+1];
        strcpy(ns, s.c_str());
        Args.emplace_back(ns);
    }

    std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));

    if (!C) {
        for(int i=2; i<Args.size(); i++) {
            delete[] Args[i];
        }
        goto error;
    }

    const driver::JobList &Jobs = C->getJobs();

    if (Jobs.size() == 0) {
        printf("no valid job\n");
        for(int i=2; i<Args.size(); i++) {
            delete[] Args[i];
        }
        goto error;
    }

    const driver::Command &Cmd = cast<driver::Command>(*Jobs.begin());

    assert(llvm::StringRef(Cmd.getCreator().getName()) == "clang");

    if (Jobs.size() > 1) {
        if (Jobs.size() != 2) {
            printf("too many jobs\n");
            for(int i=2; i<Args.size(); i++) {
                delete[] Args[i];
            }
            goto error;
        }
        const driver::Command &Cmd2 = cast<driver::Command>(*(++Jobs.begin()));
        assert(llvm::StringRef(Cmd2.getCreator().getName()) == "GNU::Linker");
    }

    // Initialize a compiler invocation object from the clang (-cc1) arguments.
    const driver::ArgStringList &CCArgs = Cmd.getArguments();
    CompilerInvocation::CreateFromArgs(*CI,
                                     const_cast<const char **>(CCArgs.data()),
                                     const_cast<const char **>(CCArgs.data()) + CCArgs.size(),
                                     *Diags.get());
    CI->getFrontendOpts().DisableFree = false;
    CompilerInstance Clang;
    Clang.setInvocation(std::move(CI));

    // Create the compilers actual diagnostics engine.
    Clang.setDiagnostics(Diags.release());

    if (!Clang.ExecuteAction(Act)) {
        for(int i=2; i<Args.size(); i++) {
            delete[] Args[i];
        }
        goto error;
    }

    llvm::Module* M = Act.takeModule().release();
    if (M == nullptr) {
        for(int i=2; i<Args.size(); i++) {
            delete[] Args[i];
        }
        goto error;
    }

    for(llvm::Function& f: M->functions()) {
        f.removeFnAttr(llvm::Attribute::AttrKind::OptimizeNone);
    }

    for(int i=2; i<Args.size(); i++) {
        delete[] Args[i];
    }
    return std::shared_ptr<llvm::Module>(M, [=](llvm::Module* lm){
        delete context;
        });
    }

    error:
    delete context;
    return nullptr;
}

llvm::PassRegistry* PRegistry;

extern "C" {
  int signal_types[] = {
      SIGILL,
      SIGTRAP,
      SIGABRT,
      SIGBUS,
      SIGFPE,
      SIGSEGV,
      SIGSYS,
      SIGINT
  };

  jmp_buf buffer;
  static char stack[SIGSTKSZ];

  void handler(int signal_code){

    longjmp(buffer, signal_code);
  }

  void segfault_handler(int signal, siginfo_t *si, void *arg) {
    assert(signal == SIGSEGV);
    int ret = (int)(size_t)si->si_addr;
    if (ret == 0) {
      ret = 0xDEADBEEF;
    }
    longjmp(buffer, ret);
  }

  void set_signal(int sig, void(*handle)(int)) {
    struct sigaction sa;

    sigset_t block_mask;

    sigemptyset (&block_mask);
    sa.sa_handler = handle;
    sa.sa_mask = block_mask;
    sa.sa_flags = SA_NODEFER;

    if (sigaction(sig, &sa, NULL) == -1) {
      perror("Error: cannot handle signal");
    }
  }

  void set_segfault_signal(void(*segfault_handler)(int, siginfo_t*, void*)) {
  stack_t ss;
  ss.ss_size = SIGSTKSZ;
  ss.ss_sp = stack;


    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sigaltstack(&ss, 0);
    sa.sa_sigaction = segfault_handler;
    sa.sa_flags   = SA_SIGINFO | SA_NODEFER | SA_ONSTACK;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
      perror("Error: cannot handle signal");
    }
  }
}

bool applyOptLevel(llvm::Module* M, int level) {
    int r = setjmp(buffer);

    if (r == 0) {
        // Assumes only segfault can happen when applying optimization
        set_segfault_signal(segfault_handler);
        try{
          llvm::PassManagerBuilder pmb;
          pmb.OptLevel = level;

          llvm::legacy::PassManager Passes;
          std::unique_ptr<llvm::legacy::FunctionPassManager> FPasses(new llvm::legacy::FunctionPassManager(M));

          pmb.populateModulePassManager(Passes);
          pmb.populateFunctionPassManager(*FPasses);

          Passes.add(llvm::createVerifierPass());
          FPasses->doInitialization();
          for (llvm::Function &F : *M)
            FPasses->run(F);
          FPasses->doFinalization();

          Passes.run(*M);
          Passes.add(llvm::createVerifierPass());
        } catch(std::exception& e) {
            std::cerr << "Exception caught : " << e.what() << std::endl;
            return false;
        }
        return true;
    } else {
        std::cerr << "Caught segfault at address " << (void*)(size_t)r << std::endl;
        return false;
    }
}

int main(int argc, char** argv) {
  int numiters = atol(argv[1]);
  printf("running for %d iters\n", numiters);


  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  // Initialize passes
  PRegistry = llvm::PassRegistry::getPassRegistry();
  llvm::initializeCore(*PRegistry);
  llvm::initializeCoroutines(*PRegistry);
  llvm::initializeScalarOpts(*PRegistry);
  llvm::initializeObjCARCOpts(*PRegistry);
  llvm::initializeVectorization(*PRegistry);
  //llvm::initializeTapirOpts(*PRegistry);
  llvm::initializeIPO(*PRegistry);
  llvm::initializeAnalysis(*PRegistry);
  llvm::initializeTransformUtils(*PRegistry);
  llvm::initializeInstCombine(*PRegistry);
  llvm::initializeInstrumentation(*PRegistry);
  llvm::initializeTarget(*PRegistry);
  // For codegen passes, only passes that do IR to IR transformation are
  // supported.
  llvm::initializeScalarizeMaskedMemIntrinPass(*PRegistry);
  llvm::initializeCodeGenPreparePass(*PRegistry);
  llvm::initializeAtomicExpandPass(*PRegistry);
  llvm::initializeRewriteSymbolsLegacyPassPass(*PRegistry);
  llvm::initializeWinEHPreparePass(*PRegistry);
  llvm::initializeDwarfEHPreparePass(*PRegistry);
  llvm::initializeSafeStackLegacyPassPass(*PRegistry);
  llvm::initializeSjLjEHPreparePass(*PRegistry);
  llvm::initializePreISelIntrinsicLoweringLegacyPassPass(*PRegistry);
  llvm::initializeGlobalMergePass(*PRegistry);
  llvm::initializeInterleavedAccessPass(*PRegistry);
  llvm::initializeCountingFunctionInserterPass(*PRegistry);
  llvm::initializeUnreachableBlockElimLegacyPassPass(*PRegistry);
  llvm::initializeExpandReductionsPass(*PRegistry);

  std::vector<std::string> args = {"-I", "/home/wmoses/git/Auto-Phase/benchmarks/polybench-c-3.2/utilities", "-include", "/home/wmoses/git/Auto-Phase/benchmarks/polybench-c-3.2/utilities/polybench.c", "-DSMALL_DATASET"};
  for(int i=0; i<numiters; i++) {
    auto g = getLLVM("/home/wmoses/git/Auto-Phase/benchmarks/polybench-c-3.2/linear-algebra/kernels/2mm/2mm.c", args);
    applyOptLevel(g.get(), 3);
  }
  //llvm::llvm_shutdown();
}
