cmake_minimum_required(VERSION 3.25)
project(BinaryEngine VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;Dist" CACHE STRING "Available build types" FORCE)
foreach(flags_variable IN ITEMS CMAKE_CXX_FLAGS CMAKE_C_FLAGS CMAKE_EXE_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS CMAKE_RC_FLAGS CMAKE_SHARED_LINKER_FLAGS CMAKE_STATIC_LINKER_FLAGS)
    set(${flags_variable}_DIST "${${flags_variable}_RELEASE}")
endforeach()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(ProjectWarnings INTERFACE)

if(MSVC)
    add_compile_options(/MP)
    target_compile_options(ProjectWarnings INTERFACE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(ProjectWarnings INTERFACE -Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers)
endif()

add_subdirectory(vendor)
include(projects.cmake)