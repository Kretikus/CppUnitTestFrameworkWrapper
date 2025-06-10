#
#  Install:
#   Boost: system, filesystem
#
cmake_minimum_required(VERSION 3.08)

project (CppUnitTestFrameworkWrapper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# so that the dynamic test libs can be created
set(CMAKE_CXX_FLAGS -fPIC)

#Comment out for debugging compile problems
#set(CMAKE_VERBOSE_MAKEFILE ON)


include_directories(${PROJECT_SOURCE_DIR}/include)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)

find_package(Boost 1.67 REQUIRED COMPONENTS system filesystem)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID REQUIRED uuid)

add_subdirectory (testhost)
add_subdirectory (test)
