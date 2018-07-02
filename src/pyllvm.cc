#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


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

namespace py = pybind11;
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

class PassLister : public llvm::PassRegistrationListener {
public:
	std::list<const llvm::PassInfo*> passes;
	void passEnumerate(const llvm::PassInfo* pi) override {
		passes.push_back(pi);
	}
	void passRegistered(const llvm::PassInfo* pi) override {}
};

llvm::PassRegistry* PRegistry;

std::vector<const llvm::PassInfo*> getOpts() {
	PassLister lister;
	PRegistry->enumerateWith(&lister);
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

  void handler(int signal, siginfo_t *si, void *arg) {
    int ret = 0;
    if (signal == SIGSEGV) {
        ret = (int)(size_t)si->si_addr;
    }    
    if (ret == 0) {
      ret = 0xDEADBEEF;
    }
    longjmp(buffer, ret);
  }

  void set_signal(void(*handler)(int, siginfo_t*, void*)) {
    stack_t ss;
    ss.ss_size = SIGSTKSZ;
    ss.ss_sp = stack;

    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sigaltstack(&ss, 0);
    sa.sa_sigaction = handler;
    sa.sa_flags   = SA_SIGINFO | SA_NODEFER | SA_ONSTACK;
  
    for(int i=0; i<sizeof(signal_types)/sizeof(*signal_types); i++) {
        if (sigaction(signal_types[i], &sa, NULL) == -1) {
          perror("Error: cannot handle signal");
        }
    }
  }
}

void llvm_handler(void*, const std::string& reason, bool gencrash) {
    std::cout << "llvm error: " << reason << "\n";
    int ret = 0xDEADBEED;
    longjmp(buffer, ret);
}

bool applyOptLevel(llvm::Module* M, int level) {
    int r = setjmp(buffer);

    if (r == 0) {
        set_signal(handler);
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

bool applyOpt(const llvm::PassInfo* pi, llvm::Module* M) {
    int r = setjmp(buffer);

    if (r == 0) {
        set_signal(handler);

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
        std::cerr << "Caught segfault at address " << (void*)(size_t)r << std::endl;
        return false;
    }
}

using ListCasterBase = pybind11::detail::list_caster<std::vector<const llvm::PassInfo*>, const llvm::PassInfo*>;
namespace pybind11 { namespace detail {
template<> struct type_caster<std::vector<const llvm::PassInfo*>> : ListCasterBase {
    static handle cast(const std::vector<const llvm::PassInfo*> &src, return_value_policy, handle parent) {
        return ListCasterBase::cast(src, return_value_policy::reference, parent);
    }
    static handle cast(const std::vector<const llvm::PassInfo*> *src, return_value_policy pol, handle parent) {
        return cast(*src, pol, parent);
    }
};
}}

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

    auto opts = new DiagnosticOptions;
    TextDiagnosticPrinter printer(llvm::errs(), opts);
    DiagnosticsEngine Diags(IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs), opts, &printer, false);
	Driver TheDriver(CLANG_BINARY, llvm::sys::getProcessTriple(), Diags);

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


PYBIND11_MODULE(pyllvm, m) {
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

  llvm::install_fatal_error_handler(&llvm_handler);
//#ifdef LINK_POLLY_INTO_TOOLS
//  polly::initializePollyPasses(*PRegistry);
//#endif

  py::class_<llvm::Module, std::shared_ptr<llvm::Module>>(m,"Module")
    .def("dump", [=](std::shared_ptr<llvm::Module> lm) {
        lm->print(llvm::dbgs(), nullptr, false, true);
        return;
    })
    .def("__str__", [=](std::shared_ptr<llvm::Module> lm) {
        std::string str;
        llvm::raw_string_ostream rso(str);
        lm->print(rso, nullptr, false, false);
        return str;
    })
    .def("clone", [=](std::shared_ptr<llvm::Module> lm) {
        return llvm::CloneModule(lm.get());
    })
    .def("timeFunction", [=](std::shared_ptr<llvm::Module> lm, std::string fn, unsigned int repeat=1) {
      AutoJIT j;
      j.addModule(llvm::CloneModule(lm.get()));

      int r = setjmp(buffer);

      double runtime = std::numeric_limits<double>::infinity();
      if (r == 0) {
        auto fun = (void (*)())j.getSymbolAddress(fn);
        set_signal(handler);

        struct timeval t1, t2;
        gettimeofday(&t1,0);
        for(int i=0; i<repeat; i++) {
          fun();
        }
        gettimeofday(&t2,0);
        unsigned long long runtime_ms = (todval(&t2)-todval(&t1))/1000;
        runtime = runtime_ms/1000.0;
      }

    //  for(int i=0; i<sizeof(signal_types)/sizeof(*signal_types); i++) {
    //    signal(signal_types[i], SIG_DFL);
    //  }
      return runtime;
    })
    .def("getStats", [=](std::shared_ptr<llvm::Module> lm, std::string fn){
      auto F = lm->getFunction(fn);
      assert(F);
      return getStats(*F);
    })
    .def("cleanup", [=](std::shared_ptr<llvm::Module> lm) {
      //auto ctx = contexts[lm.get()];
      //contexts.erase(lm.get());
      //delete ctx;
    });

  py::class_<llvm::PassInfo>(m,"PassInfo")
    .def("getPassName", [](llvm::PassInfo& pi) {
    	return std::string(pi.getPassName());
    })
    .def("getPassArgument", [](llvm::PassInfo& pi) {
    	return std::string(pi.getPassArgument());
    })
    .def("isAnalysis", &llvm::PassInfo::isAnalysis)
    ;

  py::class_<clang::ASTUnit>(m,"ASTUnit")
    .def("dump", [=](ASTUnit& ast) {
        auto ad = clang::CreateASTDumper("", true, false, false);
        ad->Initialize(ast.getASTContext());
        ad->HandleTranslationUnit(ast.getASTContext());
    });
  m.def("getLLVM", getLLVM);
  m.def("getOpts", getOpts, py::return_value_policy::take_ownership);
  m.def("applyOpt", applyOpt);
  m.def("applyOptLevel", applyOptLevel);
  m.def("createBinary", createBinary);

}
