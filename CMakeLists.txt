cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(AOC24)
set(INPUT_DIR ${PROJECT_SOURCE_DIR}/input)


#======================= INCLUSION OF Our Code ======================#
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
configure_file(${SOURCE_DIR}/DirectoryMacro.h.in ${SOURCE_DIR}/DirectoryMacro.h)

file(GLOB_RECURSE SOURCE_FILES
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
)

# add the data to the target
add_executable(${PROJECT_NAME} "${SOURCE_FILES}")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
