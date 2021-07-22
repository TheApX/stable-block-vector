cmake_minimum_required(VERSION 3.14)

project(stable_block_vector)

set(CMAKE_CXX_STANDARD 20)

# Suppress long and annoying warning.
set(GCC_NO_PSABI_WARNING "-Wno-psabi")
add_definitions(${GCC_NO_PSABI_WARNING})

# Main library: stable_block_vector
set(stable_block_vector_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stable_block_vector.hpp
)
add_library(stable_block_vector STATIC
    ${stable_block_vector_SRCS}
)
set_target_properties(stable_block_vector PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(stable_block_vector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


# Fetch googletest
find_package(Threads REQUIRED)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/master.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# Tests
enable_testing()
set(stable_block_vector_test_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/test/stable_block_vector_test.cpp
)
add_executable(stable_block_vector_test ${stable_block_vector_test_SRCS})
target_link_libraries(stable_block_vector_test
    stable_block_vector
    gtest_main
    gmock_main
)
add_test(NAME stable_block_vector_test
         COMMAND stable_block_vector_test)
