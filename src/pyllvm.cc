#include <pybind11/pybind11.h>
#include <clang-c/Index.h>
#include <string>

#include <clang/Basic/FileSystemOptions.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/PCHContainerOperations.h>


#include <llvm/ADT/STLExtras.h>

//#include <clang/tools/libclang/CIndexer.h>

namespace py = pybind11;
using namespace clang;

struct CXTranslationUnitImpl {
  void *CIdx;
  clang::ASTUnit *TheASTUnit;
  void *StringPool;
  void *Diagnostics;
  void *OverridenCursorsPool;
  void *CommentToXML;
};

clang::ASTUnit* parseC(std::string filename) {
  auto index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(
    index,
    filename.c_str(),
    /* args */ nullptr, 0,
    /* unsaved files */ nullptr, 0,
    /* options */ CXTranslationUnit_None
  );
  return unit->TheASTUnit;
}

PYBIND11_MODULE(pyllvm, m) {
  py::class_<clang::ASTUnit>(m,"ASTUnit");//, std::unique_ptr<clang::ASTUnit>>(m,"ASTUnit");
  m.def("parseC", parseC);
}
