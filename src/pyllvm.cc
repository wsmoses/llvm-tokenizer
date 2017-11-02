#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
//#include <clang-c/Index.h>
#include <string>
#include <iostream>

#include <clang/Frontend/ASTConsumers.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/PCHContainerOperations.h>
#include <clang/Lex/PreprocessorOptions.h>


#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Option/ArgList.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/PassInfo.h>

//#include <clang/tools/libclang/CIndexer.h>

namespace py = pybind11;
using namespace clang;

//===-- examples/clang-interpreter/main.cpp - Clang C Interpreter Example -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Tool.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
using namespace clang;
using namespace clang::driver;


llvm::LLVMContext llvmContext;
DiagnosticOptions DiagOpts;
DiagnosticsEngine* Diags;

std::unique_ptr<llvm::Module> getLLVM(std::string filename) {
	std::unique_ptr<llvm::Module> mod;
	EmitLLVMOnlyAction Act(&llvmContext);
  	
  	std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);

	Driver TheDriver(CLANG_BINARY, llvm::sys::getProcessTriple(), *Diags);

	SmallVector<const char *, 2> Args {"clang", filename.c_str()};
	std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));
	if (!C)
	return nullptr;

	const driver::JobList &Jobs = C->getJobs();
	assert(Jobs.size() == 1 && isa<driver::Command>(*Jobs.begin()));

	const driver::Command &Cmd = cast<driver::Command>(*Jobs.begin());
	assert(llvm::StringRef(Cmd.getCreator().getName()) == "clang");

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

	return Act.takeModule();
}

//#ifdef LINK_POLLY_INTO_TOOLS
//namespace polly {
//void initializePollyPasses(llvm::PassRegistry &Registry);
//}
//#endif

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

bool applyOpt(const llvm::PassInfo* pi, llvm::Module* M) {
  try{
	  llvm::legacy::PassManager Passes;
	  Passes.add(pi->createPass());
	  Passes.add(llvm::createVerifierPass());
	  Passes.run(*M);
	}catch(std::exception e) {
		return false;
	}
	return true;
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

PYBIND11_MODULE(pyllvm, m) {
  Diags = new DiagnosticsEngine(IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()), &DiagOpts, new TextDiagnosticPrinter(llvm::errs(), &DiagOpts));

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  //LLVMInitializeX86Target();
  //LLVMInitializeX86TargetMC();
  //LLVMInitializeX86AsmPrinter();
  //LLVMInitializeX86AsmParser();

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

//#ifdef LINK_POLLY_INTO_TOOLS
//  polly::initializePollyPasses(*Registry);
//#endif

  py::class_<llvm::Module>(m,"Module")
    .def("dump", [=](llvm::Module& lm) { 
        lm.print(llvm::dbgs(), nullptr, false, true);
        return;
    })
    .def("__str__", [=](llvm::Module& lm) {
        std::string str;
        llvm::raw_string_ostream rso(str);
        lm.print(llvm::dbgs(), nullptr, false, false);
        return str;
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
  m.def("createBinary", createBinary);
}
