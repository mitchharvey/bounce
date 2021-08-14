cmake_minimum_required(VERSION 3.10)

project(Bounce)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(Bounce main.cpp)

target_link_libraries(Bounce ${CURSES_LIBRARIES})
