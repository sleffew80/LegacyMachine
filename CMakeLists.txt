# CMakeList.txt : Top-level LegacyMachine CMake project file, do global
#
# LegacyMachine - A libRetro implementation to be used as a stand-alone 
# launcher or to build as a library for creating simple lo-fi frontends intended 
# to simulate the look and feel of the classic video gaming consoles, computers, 
# and arcade machines being emulated.
#
cmake_minimum_required (VERSION 3.23)

#----------------------------------------------------------------------------------------------------------------------
# Project
#----------------------------------------------------------------------------------------------------------------------
project ("LegacyMachine")

#----------------------------------------------------------------------------------------------------------------------
# User Configuration
#----------------------------------------------------------------------------------------------------------------------

# Define user configurable options.
option(BUILD_EXAMPLES "Build Example Projects." ON)
option(EXCLUDE_7ZIP "Exclude 7-Zip support." OFF)

#----------------------------------------------------------------------------------------------------------------------
# Global Configuration
#----------------------------------------------------------------------------------------------------------------------

# Set environment paths
if(NOT WIN32)
  if(DEFINED ENV{XDG_CONFIG_HOME})
    set(USER_HOME_DIRECTORY_PATH $ENV{XDG_CONFIG_HOME})
    set(HAS_XDG_CONFIG_PATH ON)
  elseif(DEFINED ENV{HOME})
    set(USER_HOME_DIRECTORY_PATH $ENV{HOME})
  endif()
endif()

# Set platform variables
if(APPLE AND UNIX)
  set(MACOSX ON)
endif()
if(UNIX AND NOT APPLE)
  set(LINUX ON)
endif()

# Check for NEON Capabilities
include(CheckSourceCompiles)
include(CheckIncludeFile)

check_include_file(arm_neon.h HAS_NEON_H)

if(HAS_NEON_H)
  check_source_compiles(C
  [=[
  #include "arm_neon.h"
  int main(void){
    float32x4_t v1 = { 1.0, 2.0, 3.0, 4.0 };
    return 0;
  }
  ]=]
  HAVE_NEON
  )
endif()

# Check for presence of a thread library.
find_package(Threads)

# Find required packages.
find_package(ZLIB REQUIRED)

if(MACOSX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PNG REQUIRED IMPORTED_TARGET libpng16)
else()
  find_package(PNG REQUIRED)
endif()

find_package(SDL3)
find_package(SDL2)

if(NOT SDL3_FOUND AND NOT SDL2_FOUND AND NOT SDL_FOUND)
    message(FATAL_ERROR "No SDL version found (SDL3, SDL2, or SDL1 required).")
endif()

# Find required libraries.
if(MACOSX)
  find_library(COCOA_LIBRARY Cocoa)
endif()

# Set conditional variables.
if(Threads_FOUND)
  set(HAVE_THREADS ON)
endif()

if(ZLIB_FOUND)
  set(HAVE_ZLIB ON)
endif()

if(PNG_FOUND)
  set(HAVE_PNG ON)
endif()

if(SDL3_FOUND)
  set(HAVE_SDL3 ON)
  set(HAVE_SDL ON)
  set(HAVE_SDL_COMMON ON)
elseif(SDL2_FOUND)
  set(HAVE_SDL2 ON)
  set(HAVE_SDL ON)
  set(HAVE_SDL_COMMON ON)
endif()

if(MACOSX)
  set(HAVE_GETOPT_LONG ON)
  set(HAVE_STRCASESTR ON)
  set(HAVE_STRL ON)
endif()
if(UNIX)
  
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(IS_DEBUG ON)
endif()

# Set dependency availability.
if(NOT EXCLUDE_7ZIP)
  set(HAVE_7ZIP ON)
endif()

if(HAVE_ZLIB OR HAVE_7ZIP)
  set(HAVE_COMPRESSION ON)
endif()

# Set dependency source and include directories.
set(DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/source/Dependencies")

set(SEVENZIP_SOURCE_DIR "${DEPENDENCIES_DIR}/7zip")

# Set the legacy machine source directory.
set(RETRO_SOURCE_DIR "${CMAKE_SOURCE_DIR}/source/LegacyMachine")

# Set the libretro source and include directories.
set(LIBRETRO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/source/Libretro/include")
set(LIBRETRO_SOURCE_DIR "${CMAKE_SOURCE_DIR}/source/Libretro")
set(LIBRETRO_INCLUDE_DIRS ${LIBRETRO_INCLUDE_DIR} ${LIBRETRO_SOURCE_DIR})

# Set Tilengine's include directory.
set(TILENGINE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/source/Tilengine/include")

# Set Tilengine to exclude windowing code.
set(TLN_EXCLUDE_WINDOW ON)

#----------------------------------------------------------------------------------------------------------------------
# Sub-projects
#----------------------------------------------------------------------------------------------------------------------

# Include sub-projects.
add_subdirectory ("source/LegacyMachine")
add_subdirectory ("source/Tilengine")

if(BUILD_EXAMPLES)
  # Include example projects.
  add_subdirectory ("source/Examples/VirtualMachine")
  add_subdirectory ("source/Examples/VirtualConsole")
endif()
