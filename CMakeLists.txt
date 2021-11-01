cmake_minimum_required(VERSION 3.5)

project("events_prototype")

set(CMAKE_C_EXTENSIONS On)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include_directories("commons/src")

add_subdirectory("libutils")
add_subdirectory("libevent")
add_subdirectory("libmainapp")
add_subdirectory("libgui")
add_subdirectory("app")
