cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(hide_completed_packs VERSION 1.0.0 LANGUAGES CXX)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable is not set!")
endif()

include("$ENV{GEODE_SDK}/cmake/GeodeFile.cmake")
add_subdirectory("$ENV{GEODE_SDK}" geode)

add_library(hide_completed_packs SHARED src/main.cpp)
setup_geode_mod(hide_completed_packs)
