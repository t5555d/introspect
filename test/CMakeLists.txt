cmake_minimum_required(VERSION 2.8.7)

project(introspect_test CXX)

include_directories(../googletest/googletest/include)

file(GLOB_RECURSE HEADERS *.h)
file(GLOB_RECURSE SOURCES *.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} gtest gtest_main introspect)
target_include_directories(${PROJECT_NAME} PUBLIC ../include)
