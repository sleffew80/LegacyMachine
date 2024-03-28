# CMakeList.txt : Top-level LegacyMachine CMake project file, do global
#
# LegacyMachine - A libRetro implementation for creating simple lo-fi 
# frontends intended to simulate the look and feel of the classic 
# video gaming consoles, computers, and arcade machines being emulated.
#
# configuration and include sub-projects here.
#
cmake_minimum_required (VERSION 3.23)

#----------------------------------------------------------------------------------------------------------------------
# Project
#----------------------------------------------------------------------------------------------------------------------
project ("LegacyMachine")

#----------------------------------------------------------------------------------------------------------------------
# Global Configuration
#----------------------------------------------------------------------------------------------------------------------

# Specify the C and C++ standards.
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Add custom local modules for Windows to the module path.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

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

# Define user configurable options.
option(BUILD_EXAMPLES "Build Example Projects." ON)

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

# Find required packages.
find_package(ZLIB REQUIRED)

if(MACOSX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PNG REQUIRED IMPORTED_TARGET libpng16)
else()
  find_package(PNG REQUIRED)
endif()

find_package(SDL2 REQUIRED)

# Find optional packages.
find_package(OpenGL)
find_package(OpenGLES)

# Find required libraries.
if(MACOSX)
  find_library(COCOA_LIBRARY Cocoa)
endif()

# Set conditional variables.
if(ZLIB_FOUND)
  set(HAVE_ZLIB ON)
endif()

if(PNG_FOUND)
  set(HAVE_PNG ON)
endif()

if(SDL2_FOUND)
  set(HAVE_SDL2 ON)
endif()

if(OpenGL_OpenGL_FOUND)
  set(HAVE_OPENGL ON)
endif()

if(OpenGLES_V2_FOUND)
  set(HAVE_OPENGLES2 ON)
endif()

if(OpenGLES_V3_FOUND)
  set(HAVE_OPENGLES3 ON)
endif()

if(OpenGLES_V32_FOUND OR OpenGLES_V31_FOUND)
  set(HAVE_OPENGLES_VERSION ON)
endif()

if(MACOSX)
  set(HAVE_GETOPT_LONG ON)
  set(HAVE_STRCASESTR ON)
  set(HAVE_STRL ON)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(IS_DEBUG ON)
endif()

set(HAVE_MENU ON)

# Set dependency source and include directories.
if (HAVE_OPENGL)
    set(GLAD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/source/Dependencies/GLAD/include")
    set(GLAD_SOURCE_DIR "${CMAKE_SOURCE_DIR}/source/Dependencies/GLAD/source")
endif()

# Set the libretro source and include directories.
set(LIBRETRO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/source/Libretro/include")
set(LIBRETRO_SOURCE_DIR "${CMAKE_SOURCE_DIR}/source/Libretro")
set(LIBRETRO_INCLUDE_DIRS ${LIBRETRO_INCLUDE_DIR} ${LIBRETRO_SOURCE_DIR})
set(LIBRETRO_COMMON_DEFINE_FLAGS "RARCH_INTERNAL")

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
  add_subdirectory ("source/Examples/VirtualFamicom")
endif()
