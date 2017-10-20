cmake_minimum_required(VERSION 2.1)

project("autophase")
set(AUTOPHASE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(AUTOPHASE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

if(NOT EXISTS ${LLVM_PREFIX}/bin/llvm-config)
  message(FATAL_ERROR "Missing: " ${LLVM_PREFIX}/bin/llvm-config)
  return()
endif()
execute_process(COMMAND ${LLVM_PREFIX}/bin/llvm-config --version
                OUTPUT_VARIABLE CLANG_VERSION)
if (NOT CLANG_VERSION EQUAL 3.9.1)
  message(STATUS  " Must use LLVM 3.9.1, but found " ${CLANG_PREFIX}/bin/llvm-config with version " " ${CLANG_VERSION})
  return()
endif()

add_subdirectory(third-party/pybind11)
pybind11_add_module(pyllvm pyllvm.cc)
install(
    TARGETS
    pyllvm
    DESTINATION
    pyllvm
)
