cmake_minimum_required(VERSION 3.0`)

project("autophase")
set(AUTOPHASE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(AUTOPHASE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

if(EXISTS ${LLVM_PREFIX}/bin/llvm-config)
  set(LLVM_DIR, ${LLVM_PREFIX}/lib/cmake/llvm)
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

add_subdirectory(third-party/pybind11)
pybind11_add_module(pyllvm src/pyllvm.cc)
target_link_libraries(pyllvm
    PRIVATE
    clangAnalysis
    clangAST
    clangBasic
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
    LLVMOption
    LLVMSupport
    tinfo
)

# Link against LLVM libraries
target_link_libraries(pyllvm PRIVATE ${llvm_libs})

