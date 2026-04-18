cmake_minimum_required(VERSION 3.21)

project(ShowcaseMod VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK not set")
endif()


include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/nlohmann)  # Add the path to nlohmann/json.hpp

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_subdirectory($ENV{GEODE_SDK} ${CMAKE_BINARY_DIR}/geode)

# Collect source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)


add_library(${PROJECT_NAME} SHARED ${SOURCES})


setup_geode_mod(${PROJECT_NAME})