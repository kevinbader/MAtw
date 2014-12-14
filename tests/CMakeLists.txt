## 
## Copyright 2014 Kevin Bader.
## Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
## 
file(GLOB TESTS_CPP "test*.cpp")
add_executable(all_tests ${TESTS_CPP})
target_link_libraries(all_tests libd2 gtest gtest_main gmock ${SQLITE3_LIBRARY_DEBUG})
add_test(all_tests all_tests)

add_custom_command(TARGET all_tests
                   COMMENT "Run tests"
                   POST_BUILD COMMAND ctest ARGS --output-on-failure
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    # coverage module from cmake directory:
    include(CodeCoverage)
    setup_target_for_coverage(coverage all_tests coverage_results)
endif(CMAKE_BUILD_TYPE STREQUAL Debug)
