cmake_minimum_required(VERSION 3.29)
project(nicotine)
set(CMAKE_CXX_STANDARD 20)
set(appname nico)


include_directories(include)
file(GLOB sources CMAKE_CONFIGURE_DEPENDS src/*.cpp)

set(sources src/main.cpp src/parseTree.cpp src/tokenize.cpp)

add_executable(${appname} ${sources})