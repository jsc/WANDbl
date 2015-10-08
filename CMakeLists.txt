cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(AppendCompilerFlags)
include(ExternalProject)

project(WAND CXX C)

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/include
                    ${CMAKE_HOME_DIRECTORY}/external/fastpfor/headers/
                    ${CMAKE_BINARY_DIR}/external/sdsl-lite/include
                    ${CMAKE_BINARY_DIR}/external/sdsl-lite/external/libdivsufsort-2.0.1/include
                    )

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/external/sdsl-lite/lib
					)

#append_cxx_compiler_flags("-msse4.2 -std=c++11 -O3 -funroll-loops -DNDEBUG -stdlib=libc++" "CLANG" CMAKE_CXX_FLAGS)
append_cxx_compiler_flags("-O3 -msse4.2 -std=c++11 -O3 -DNDEBUG -fforce-addr -fomit-frame-pointer -funroll-loops -frerun-cse-after-loop -frerun-loop-opt -march=native" "GCC" CMAKE_CXX_FLAGS)
#append_cxx_compiler_flags("-O -g -W -Wall -msse4.2 -std=c++11" "GCC" CMAKE_CXX_FLAGS)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall ")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math -DNDEBUG -funroll-loops")


ADD_SUBDIRECTORY(external/sdsl-lite)

ADD_LIBRARY(fastpfor_lib STATIC external/fastpfor/src/bitpacking.cpp
                                external/fastpfor/src/bitpackingaligned.cpp
                                external/fastpfor/src/bitpackingunaligned.cpp
                                external/fastpfor/src/simdunalignedbitpacking.cpp
                                external/fastpfor/src/simdbitpacking.cpp)

  ADD_EXECUTABLE(wand_search src/wand_search.cpp)
  TARGET_LINK_LIBRARIES(wand_search sdsl divsufsort divsufsort64 pthread fastpfor_lib)

