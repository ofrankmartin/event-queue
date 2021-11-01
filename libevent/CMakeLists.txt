cmake_minimum_required(VERSION 3.5)

set(TARGET_NAME zzevent)

set(CMAKE_C_EXTENSIONS On)

project(${TARGET_NAME} C)

file(GLOB SOURCES "src/*.c")

add_library(${TARGET_NAME} STATIC ${SOURCES})

target_include_directories("${TARGET_NAME}" PUBLIC "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(${TARGET_NAME} utils)
