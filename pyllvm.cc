#include <pybind11/pybind11.h>
#include <clang-c/Index.h>
#include <string>

namespace py = pybind11;

PYBIND11_PLUGIN(pyllvm) {
  py::module m("pyllvm", "Python LLVM Bindings");

  m.def("parseC", [](std::string filename) {
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
      index,
      filename,
      /* args */ nullptr, 0,
      /* unsaved files */ nullptr, 0,
      /* options */ CXTranslationUnit_None
    );
    return unit;
  });

  

}
