#include <pybind11/pybind11.h>
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
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>

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

#define STRING(s) #s

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
std::string GetExecutablePath(const char *Argv0) {
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::fs::getMainExecutable(Argv0, MainAddr);
}

llvm::LLVMContext llvmContext;
DiagnosticOptions DiagOpts;
DiagnosticsEngine* Diags;

std::unique_ptr<llvm::Module> getLLVM(std::string filename) {
	std::unique_ptr<llvm::Module> mod;
	EmitLLVMOnlyAction Act(&llvmContext);
  	
  	std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);

  	std::string PythonPath = llvm::sys::fs::getMainExecutable(STRING(CLANG_BINARY), (void*) (intptr_t)getLLVM);
	Driver TheDriver(PythonPath, llvm::sys::getProcessTriple(), *Diags);

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


PYBIND11_MODULE(pyllvm, m) {
  Diags = new DiagnosticsEngine(IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()), &DiagOpts, new TextDiagnosticPrinter(llvm::errs(), &DiagOpts));

  py::class_<llvm::Module>(m,"Module")
    .def("dump", [=](llvm::Module& lm) { 
    	std::cerr << "dump lm " << (void*)&lm << std::endl;
        lm.print(llvm::dbgs(), nullptr, false, true);
        return;
    });
  py::class_<clang::ASTUnit>(m,"ASTUnit")
    .def("printStats", [=](ASTUnit& ast) {
        ast.getASTContext().PrintStats();
        return;
    }).def("dump", [=](ASTUnit& ast) {
        auto ad = clang::CreateASTDumper("", true, false, false);
        ad->Initialize(ast.getASTContext());
        ad->HandleTranslationUnit(ast.getASTContext());
    });
  m.def("getLLVM", getLLVM);
}
