## 
## Copyright 2014 Kevin Bader.
## Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
## 
cmake_minimum_required(VERSION 2.8)

project(d2 C CXX)
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_definitions(
    -std=c++11
    -Wall
    -Wextra
    -Woverloaded-virtual
    -march=native
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    set(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
    set(GCC_COVERAGE_LIBS          "gcov")
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")


## GTEST
add_subdirectory(gmock-1.7.0)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})


include_directories(src lib/sqlite)
subdirs(lib src tests)


find_package(Boost 1.49 COMPONENTS system program_options filesystem regex REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set(MA3_RELEASE 1)
add_executable(MA3_-${MA3_RELEASE} MA3.cpp config.cpp)
set_target_properties(MA3_-${MA3_RELEASE} PROPERTIES COMPILE_FLAGS "-DVARIANT='(MA3|MA3_)' -DRELEASE=${MA3_RELEASE}")

set(MA2_RELEASE 1)
add_executable(MA2_-${MA2_RELEASE} MA2.cpp config.cpp)
set_target_properties(MA2_-${MA2_RELEASE} PROPERTIES COMPILE_FLAGS "-DVARIANT='(MA2|MA2_)' -DRELEASE=${MA2_RELEASE}")

set(MA1_RELEASE 1)
add_executable(MA1_-${MA1_RELEASE} MA1.cpp config.cpp)
set_target_properties(MA1_-${MA1_RELEASE} PROPERTIES COMPILE_FLAGS "-DVARIANT='(MA1|MA1_)' -DRELEASE=${MA1_RELEASE}")

set(VARIANTS MA3_-${MA3_RELEASE} MA2_-${MA2_RELEASE} MA1_-${MA1_RELEASE})
foreach(variant ${VARIANTS})
    target_link_libraries(${variant} libd2 ${GCC_COVERAGE_LIBS})
    install(TARGETS ${variant} RUNTIME DESTINATION bin)
endforeach(variant)
