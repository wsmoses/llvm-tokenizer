cmake_minimum_required(VERSION 3.0`)

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
execute_process(COMMAND ${LLVM_PREFIX}/bin/llvm-config --includedir
OUTPUT_VARIABLE LLVM_INCDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(${LLVM_INCDIR})

execute_process(COMMAND ${LLVM_PREFIX}/bin/llvm-config --libdir
OUTPUT_VARIABLE LLVM_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
link_directories(${LLVM_LIBDIR})

add_subdirectory(third-party/pybind11)
pybind11_add_module(pyllvm src/pyllvm.cc)
target_link_libraries(pyllvm PRIVATE clang)
#clangParse clangCodeGen)

