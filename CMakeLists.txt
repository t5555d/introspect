cmake_minimum_required(VERSION 2.8.7)

project(introspect CXX)

include_directories(include)

file(GLOB_RECURSE HEADERS src/*.h include/*.h)
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
