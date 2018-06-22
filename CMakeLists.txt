cmake_minimum_required(VERSION 3.0`)

project("autophase")
set(AUTOPHASE_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
set(AUTOPHASE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_CXX_STANDARD 11)

message(STATUS "source dir ${AUTOPHASE_SOURCE_DIR}")
message(STATUS "binary dir ${AUTOPHASE_BINARY_DIR}")

message(STATUS "GIVEN LLVM PREFIX ${LLVM_PREFIX}")

if(EXISTS ${LLVM_PREFIX}/bin/llvm-config)
  set(LLVM_DIR ${LLVM_PREFIX}/lib/cmake/llvm)
  message(STATUS "setting LLVM_DIR from prefix ${LLVM_DIR}")
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

if (NOT LLVM_PACKAGE_VERSION EQUAL 5.0.0)
  message(STATUS  " Must use LLVM 5.0.0, but found " ${LLVM_PACKAGE_VERSION})
  return()
endif()

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader) #clang)

add_definitions(-DCLANG_BINARY="${LLVM_TOOLS_BINARY_DIR}/clang")
#set(CMAKE_EXE_LINKER_FLAGS " -static")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third-party/pybind11)
pybind11_add_module(pyllvm src/pyllvm.cc)

file(GLOB libs_llvmclang "${LLVM_LIBRARY_DIRS}/libLLVM*.a" "${LLVM_LIBRARY_DIRS}/libclang*.a")

message(STATUS "finding llvm libs: " ${libs_llvmclang})

foreach(var ${libs_llvmclang})
get_filename_component(comp ${var} NAME_WE)
string(SUBSTRING ${comp} 3 -1 comp2)
list(APPEND trimmedlist ${comp2})
endforeach()

message(STATUS "finding llvm libs: " ${trimmedlist})

add_executable (pyllvm_test src/pyllvm_test)
target_link_libraries(pyllvm_test
    PRIVATE
    "-Wl,--start-group"
    ${trimmedlist}
    "-Wl,--end-group"
)

target_link_libraries(pyllvm
    PRIVATE
    "-Wl,--start-group"
    ${trimmedlist}
    "-Wl,--end-group"
)


#target_link_libraries(pyllvm
#    PRIVATE
#    clang
#    clangAnalysis
#    clangAST
#    clangBasic
#    clangCodeGen
#    clangDriver
#    clangEdit
#    clangFrontend
#    clangLex
#    clangParse
#    clangSema
#    clangSerialization
#    LLVMAnalysis
#    LLVMBitReader
#    LLVMBinaryFormat
#    LLVMCore
#    LLVMCoroutines
#    LLVMCoverage
#    LLVMLTO
#    #LLVMLanaiDesc
#    LLVMMCJIT
#    LLVMOption
#    LLVMSupport
#    LLVMX86AsmParser
#    LLVMX86CodeGen
#    LLVMX86Desc
#    LLVMX86Info
#    tinfo
#)

# Link against LLVM libraries
#target_link_libraries(pyllvm PRIVATE ${llvm_libs})

