cmake_minimum_required(VERSION 3.29)
project(external_aimassist)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

set(MSVC_LIKE OFF)
if (MSVC)
    set(MSVC_LIKE ON)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
        set(MSVC_LIKE ON)
    endif ()
endif ()

if (MSVC_LIKE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    add_compile_options(/MP /FS /utf-8 /bigobj /GR /nologo)
else()
    add_compile_options(-fno-rtti)
endif()

file(GLOB_RECURSE sourceFiles CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(external_aimassist WIN32 ${sourceFiles})
target_include_directories(external_aimassist PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_precompile_headers(external_aimassist PRIVATE src/PCH.hpp)
target_link_libraries(external_aimassist PRIVATE Version Psapi)
