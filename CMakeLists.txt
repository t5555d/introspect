cmake_minimum_required(VERSION 2.8.7)
project(introspect)

add_subdirectory(src)

set(gtest_force_shared_crt ON)
set(BUILD_GMOCK OFF)
add_subdirectory(googletest)
add_subdirectory(test)
