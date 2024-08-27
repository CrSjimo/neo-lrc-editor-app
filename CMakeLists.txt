cmake_minimum_required(VERSION 3.17)

project(NeoLrcEditorApp VERSION 0.0.1.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# ----------------------------------
# Build Options
# ----------------------------------
option(APP_BUILD_TRANSLATIONS "Build translations" ON)
option(APP_BUILD_TESTS "Build test cases" OFF)
option(APP_INSTALL "Install library" ON)

# ----------------------------------
# CMake Settings
# ----------------------------------
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX "d")
    endif()
endif()

if(APP_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

# ----------------------------------
# Project Variables
# ----------------------------------
set(APP_VERSION ${PROJECT_VERSION})
set(APP_INSTALL_NAME ${PROJECT_NAME})
set(APP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ----------------------------------
# Find basic dependencies
# ----------------------------------
find_package(qmsetup REQUIRED)

qm_import(Filesystem Deploy)
qm_init_directories()

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(src)

add_subdirectory(lib)

if(APP_BUILD_TESTS)
    add_subdirectory(tests)
endif()