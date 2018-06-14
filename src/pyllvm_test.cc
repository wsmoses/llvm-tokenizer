#include <iostream>
#include <memory>
#include <string>

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


llvm::LLVMContext llvmContext;
DiagnosticOptions DiagOpts;
DiagnosticsEngine* Diags;

std::unique_ptr<llvm::Module> getLLVM(std::string filename, std::vector<std::string> addl_args) {
    std::unique_ptr<llvm::Module> mod;
    EmitLLVMOnlyAction Act(&llvmContext);

  std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);

    Driver TheDriver(CLANG_BINARY, llvm::sys::getProcessTriple(), *Diags);

    SmallVector<const char *, 2> Args {"clang", filename.c_str()};

  for(std::string& s: addl_args) {
      Args.emplace_back(s.c_str());
  }

    std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));

  if (!C) return nullptr;

    const driver::JobList &Jobs = C->getJobs();

  if (Jobs.size() == 0) {
      printf("no valid job\n");
      return nullptr;
  }

  const driver::Command &Cmd = cast<driver::Command>(*Jobs.begin());

  assert(llvm::StringRef(Cmd.getCreator().getName()) == "clang");

  if (Jobs.size() > 1) {
      if (Jobs.size() != 2) {
          printf("too many jobs\n");
          return nullptr;
      }
      const driver::Command &Cmd2 = cast<driver::Command>(*(++Jobs.begin()));
      assert(llvm::StringRef(Cmd2.getCreator().getName()) == "GNU::Linker");
  }

    // Initialize a compiler invocation object from the clang (-cc1) arguments.
    const driver::ArgStringList &CCArgs = Cmd.getArguments();
    CompilerInvocation::CreateFromArgs(*CI,
                                     const_cast<const char **>(CCArgs.data()),
                                     const_cast<const char **>(CCArgs.data()) + CCArgs.size(),
                                     *Diags);
    CompilerInstance Clang;
    Clang.setInvocation(std::move(CI));

    // Create the compilers actual diagnostics engine.
    Clang.createDiagnostics();
    if (!Clang.hasDiagnostics())
        return nullptr;

    if (!Clang.ExecuteAction(Act))
        return nullptr;

    auto M = Act.takeModule();
  for(llvm::Function& f:M->functions()) {
      f.removeFnAttr(llvm::Attribute::AttrKind::OptimizeNone);
  }
  return M;
}

class PassLister : public llvm::PassRegistrationListener {
public:
    std::list<const llvm::PassInfo*> passes;
    void passEnumerate(const llvm::PassInfo* pi) override {
        passes.push_back(pi);
    }
    void passRegistered(const llvm::PassInfo* pi) override {}
};

llvm::PassRegistry* Registry;

std::vector<const llvm::PassInfo*> getOpts() {
    PassLister lister;
    Registry->enumerateWith(&lister);
    return std::vector<const llvm::PassInfo*>(lister.passes.begin(), lister.passes.end());
}

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
          std::unique_ptr<llvm::legacy::FunctionPassManager> FPasses;

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
        std::cerr << "Caught segfault at address " << (void*)r << std::endl;
        return false;
    }
}

bool applyOpt(const llvm::PassInfo* pi, llvm::Module* M) {
    int r = setjmp(buffer);

    if (r == 0) {
        // Assumes only segfault can happen when applying optimization
        set_segfault_signal(segfault_handler);

        try{
          llvm::legacy::PassManager Passes;
          Passes.add(pi->createPass());
          Passes.add(llvm::createVerifierPass());
          Passes.run(*M);
        } catch(std::exception& e) {
            std::cerr << "Exception caught : " << e.what() << std::endl;
            return false;
        }

        return true;
    } else {
        std::cerr << "Caught segfault at address " << (void*)r << std::endl;
        return false;
    }
}

bool createBinary(llvm::Module* M, std::string output) {
    std::string tmpfile(std::tmpnam(nullptr));

    {
        std::error_code EC;
        llvm::raw_fd_ostream dest(tmpfile.c_str(), EC, llvm::sys::fs::F_None);

        if (EC) {
          llvm::errs() << "Could not open file: " << EC.message();
          return false;
        }

        M->print(dest, nullptr, false, false);
        dest.flush();
    }


    std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);
    Driver TheDriver(CLANG_BINARY, llvm::sys::getProcessTriple(), *Diags);

    llvm::SmallVector<const char *, 6> Args {"clang", "-x", "ir", tmpfile.c_str(), "-o", output.c_str()};
    std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));
    if (!C) return false;

    llvm::SmallVector<std::pair<int, const Command *>, 2> FailingCommands;
    int Res = TheDriver.ExecuteCompilation(*C, FailingCommands);

    if (Res < 0) {
        for(auto c : FailingCommands) {
            c.second->Print(llvm::errs(), "\n", true);
        }
        return false;
    }

  return true;
}

class AutoJIT {
private:
  std::unique_ptr<llvm::TargetMachine> TM_;
  const llvm::DataLayout DL_;
  llvm::orc::RTDyldObjectLinkingLayer objectLayer_;
  llvm::orc::IRCompileLayer<decltype(objectLayer_), llvm::orc::SimpleCompiler>
    compileLayer_;
public:
  AutoJIT() :
    TM_(llvm::EngineBuilder().selectTarget()),
    DL_(TM_->createDataLayout()),
    objectLayer_([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
    compileLayer_(objectLayer_, llvm::orc::SimpleCompiler(*TM_)) {
      std::string err;
      llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr, &err);
      if (err != "") {
        throw std::runtime_error("Failed to initialize library: " + err);
      }
  }

  using ModuleHandle = decltype(compileLayer_)::ModuleHandleT;

  llvm::TargetMachine& getTargetMachine() {
    return *TM_;
  }

  ModuleHandle addModule(std::unique_ptr<llvm::Module> M) {
    M->setTargetTriple(TM_->getTargetTriple().str());
    auto Resolver = llvm::orc::createLambdaResolver(
        [&](const std::string& Name) {
          if (auto Sym = compileLayer_.findSymbol(Name, false))
            return Sym;
          return llvm::JITSymbol(nullptr);
        },
        [](const std::string& Name) {
          if (auto SymAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(Name))
            return llvm::JITSymbol(SymAddr, llvm::JITSymbolFlags::Exported);
          return llvm::JITSymbol(nullptr);
        });

    auto res = compileLayer_.addModule(std::move(M), std::move(Resolver));
    //handleErrors(res);
    assert(res && "Failed to JIT compile.");
    return *res;
  }

  /*
  void removeModule(ModuleHandle H) {
    llvm::Error res = compileLayer_.removeModule(H);
    assert(res && "Failed to remove JIT compile.");
  }
  */

  llvm::JITSymbol findSymbol(const std::string Name) {
    std::string MangledName;
    llvm::raw_string_ostream MangledNameStream(MangledName);
    llvm::Mangler::getNameWithPrefix(MangledNameStream, Name, DL_);
    return compileLayer_.findSymbol(MangledNameStream.str(), true);
  }

  llvm::JITTargetAddress getSymbolAddress(const std::string Name) {
    auto res = findSymbol(Name).getAddress();
    assert(res && "Could not find symbol");
    return *res;
  }
};

static inline unsigned long long todval (struct timeval *tp) {
    return tp->tv_sec * 1000 * 1000 + tp->tv_usec;
}


float timeFunction(llvm::Module& lm, std::string fn, unsigned int repeat=1) {
      AutoJIT j;
      j.addModule(llvm::CloneModule(&lm));

      int r = setjmp(buffer);

      double runtime = std::numeric_limits<double>::infinity();
      if (r == 0) {
        auto fun = (void (*)())j.getSymbolAddress(fn);
        for(int i=0; i<sizeof(signal_types)/sizeof(*signal_types); i++) {
          set_signal(signal_types[i], handler);
        }

        struct timeval t1, t2;
        gettimeofday(&t1,0);
        for(int i=0; i<repeat; i++) {
          fun();
        }
        gettimeofday(&t2,0);
        unsigned long long runtime_ms = (todval(&t2)-todval(&t1))/1000;
        runtime = runtime_ms/1000.0;
      }

      for(int i=0; i<sizeof(signal_types)/sizeof(*signal_types); i++) {
        signal(signal_types[i], SIG_DFL);
      }
      return runtime;

};
int main() {
  Diags = new DiagnosticsEngine(IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()), &DiagOpts, new TextDiagnosticPrinter(llvm::errs(), &DiagOpts));

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();


  // Initialize passes
  Registry = llvm::PassRegistry::getPassRegistry();
  llvm::initializeCore(*Registry);
  llvm::initializeCoroutines(*Registry);
  llvm::initializeScalarOpts(*Registry);
  llvm::initializeObjCARCOpts(*Registry);
  llvm::initializeVectorization(*Registry);
  //llvm::initializeTapirOpts(*Registry);
  llvm::initializeIPO(*Registry);
  llvm::initializeAnalysis(*Registry);
  llvm::initializeTransformUtils(*Registry);
  llvm::initializeInstCombine(*Registry);
  llvm::initializeInstrumentation(*Registry);
  llvm::initializeTarget(*Registry);
  // For codegen passes, only passes that do IR to IR transformation are
  // supported.
  llvm::initializeScalarizeMaskedMemIntrinPass(*Registry);
  llvm::initializeCodeGenPreparePass(*Registry);
  llvm::initializeAtomicExpandPass(*Registry);
  llvm::initializeRewriteSymbolsLegacyPassPass(*Registry);
  llvm::initializeWinEHPreparePass(*Registry);
  llvm::initializeDwarfEHPreparePass(*Registry);
  llvm::initializeSafeStackLegacyPassPass(*Registry);
  llvm::initializeSjLjEHPreparePass(*Registry);
  llvm::initializePreISelIntrinsicLoweringLegacyPassPass(*Registry);
  llvm::initializeGlobalMergePass(*Registry);
  llvm::initializeInterleavedAccessPass(*Registry);
  llvm::initializeCountingFunctionInserterPass(*Registry);
  llvm::initializeUnreachableBlockElimLegacyPassPass(*Registry);
  llvm::initializeExpandReductionsPass(*Registry);

  for(int i=0; i<200; i++) {
  std::vector<std::string> args = {"-I", "/home/wmoses/git/Auto-Phase/benchmarks/polybench-c-3.2/utilities", "-include", "/home/wmoses/git/Auto-Phase/benchmarks/polybench-c-3.2/utilities/polybench.c", "-DSMALL_DATASET"};
  auto g = getLLVM("/home/wmoses/git/Auto-Phase/benchmarks/polybench-c-3.2/linear-algebra/kernels/2mm/2mm.c", args);
  timeFunction(*g.get(), "main", 1);
  }
}
