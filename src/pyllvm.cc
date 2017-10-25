#include <pybind11/pybind11.h>
#include <clang-c/Index.h>
#include <string>

#include <clang/Basic/FileSystemOptions.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/PCHContainerOperations.h>

namespace py = pybind11;
using namespace clang;

std::unique_ptr<clang::ASTUnit> parseC(std::string filename) {
    FileSystemOptions fops;
    auto Diags = clang::CompilerInstance::createDiagnostics(new DiagnosticOptions());
    auto res = clang::ASTUnit::LoadFromASTFile(filename,
        RawPCHContainerReader(),
        clang::ASTUnit::WhatToLoad::LoadEverything,
        Diags,
        fops,
        /*debug info*/ false);
    return res;


  #if 0
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(
    index,
    filename.c_str(),
    /* args */ nullptr, 0,
    /* unsaved files */ nullptr, 0,
    /* options */ CXTranslationUnit_None
  );
  return unit;
  #endif
}

PYBIND11_MODULE(pyllvm, m) {
  py::class_<clang::ASTUnit, std::unique_ptr<clang::ASTUnit>>(m,"ASTUnit");
  m.def("parseC", parseC);
}
