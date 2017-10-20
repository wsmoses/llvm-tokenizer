#include <pybind11/pybind11.h>
#include <clang-c/Index.h>
#include <string>

namespace py = pybind11;


void* parseC(std::string filename) {
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(
    index,
    filename.c_str(),
    /* args */ nullptr, 0,
    /* unsaved files */ nullptr, 0,
    /* options */ CXTranslationUnit_None
  );
  return unit;
}

PYBIND11_MODULE(pyllvm, m) {
  m.def("parseC", parseC);
}
