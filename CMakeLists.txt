cmake_minimum_required(VERSION 3.0`)

project("autophase")
set(AUTOPHASE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(AUTOPHASE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

message(STATUS "GIVEN LLVM PREFIX ${LLVM_PREFIX}")

if(EXISTS ${LLVM_PREFIX}/bin/llvm-config)
  set(LLVM_DIR ${LLVM_PREFIX}/lib/cmake/llvm)
  message(STATUS "setting LLVM_DIR from prefix ${LLVM_DIR}")
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

if (NOT LLVM_PACKAGE_VERSION EQUAL 5.0.0)
  message(STATUS  " Must use LLVM 5.0.0, but found " ${LLVM_PACKAGE_VERSION)
  return()
endif()

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader) #clang)

add_definitions(-DCLANG_BINARY="${LLVM_TOOLS_BINARY_DIR}/clang")
set(CMAKE_EXE_LINKER_FLAGS " -static")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third-party/pybind11)
pybind11_add_module(pyllvm src/pyllvm.cc)
target_link_libraries(pyllvm
    PRIVATE
    clang
    clangAnalysis
    clangAST
    clangBasic
    clangCodeGen
    clangDriver
    clangEdit
    clangFrontend
    clangLex
    clangParse
    clangSema
    clangSerialization
    LLVMAnalysis
    LLVMBitReader
    LLVMBinaryFormat
    LLVMCore
    LLVMCoroutines
    LLVMCoverage
    LLVMLTO
    LLVMLanaiDesc
    LLVMMCJIT
    LLVMOption
    LLVMSupport
    LLVMX86AsmParser
    LLVMX86CodeGen
    LLVMX86Desc
    LLVMX86Info
    tinfo
)

# Link against LLVM libraries
#target_link_libraries(pyllvm PRIVATE ${llvm_libs})

