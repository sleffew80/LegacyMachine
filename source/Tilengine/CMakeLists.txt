# CMakeList.txt : Tilengine CMake project file.

cmake_minimum_required (VERSION 3.23)

#----------------------------------------------------------------------------------------------------------------------
# Project
#----------------------------------------------------------------------------------------------------------------------
project("Tilengine" VERSION 2.15.2 
		DESCRIPTION "Tilengine is a free, cross-platform 2D graphics engine for creating classic/retro games with tilemaps, sprites and palettes. Its unique scanline-based rendering system makes raster effects a core feature, a technique used by many games running on real 2D graphics chips." 
		LANGUAGES C)

#----------------------------------------------------------------------------------------------------------------------
# Configuration
#----------------------------------------------------------------------------------------------------------------------
if(NOT WIN32)
	include(GNUInstallDirs)
endif()

set(TILE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include" ${PNG_INCLUDE_DIRS})

# List all public library headers
set(PUBLIC_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/Tilengine.h")

# List all private library headers
set(TILE_HEADER_FILES "aes.h" "Animation.h" "Base64.h" "Bitmap.h" "Blitters.h" "cJSON.h"
				"crc32.h" "Debug.h" "DIB.h" "Draw.h" "Engine.h" "Layer.h" "List.h" "LoadFile.h"
				"LoadTMX.h" "Math2D.h" "md5.h" "Object.h" "ObjectList.h" "Palette.h" "ResPack.h"
				"Sequence.h" "SequencePack.h" "simplexml.h" "Sprite.h" "Spriteset.h" "Tables.h"
				"Tilemap.h" "Tileset.h")

# List all required library sources
set(TILE_SOURCE_FILES "Tilengine.c" "aes.c" "Animation.c" "Base64.c" "Bitmap.c" "Blitters.c"
				"cJSON.c" "crc32.c" "Draw.c" "Layer.c" "List.c" "LoadBitmap.c" "LoadFile.c"
				"LoadPalette.c" "LoadSequencePack.c" "LoadSpriteset.c" "LoadTilemap.c"
				"LoadTileset.c" "LoadTMX.c" "Math2D.c" "md5.c" "Object.c" "ObjectList.c"
				"Palette.c" "ResourcePacker.c" "Sequence.c" "SequencePack.c" "simplexml.c"
				"Sprite.c" "Spriteset.c" "Tables.c" "Tilemap.c" "Tileset.c" "Window.c"
				"World.c")


set(TILE_DEFINE_FLAGS "LIB_EXPORTS")
set(TILE_OPTION_FLAGS "")
set(TILE_LIBRARY_FLAGS "")

#---------------------------------------
# Build Configuration
#---------------------------------------
if(IS_DEBUG)
  set(TILE_DEFINE_FLAGS ${TILE_DEFINE_FLAGS} "_DEBUG" "DEBUG")
else()
  set(TILE_DEFINE_FLAGS ${TILE_DEFINE_FLAGS} "NDEBUG")
endif()

#---------------------------------------
# Platform Configuration
#---------------------------------------
if(WIN32)
  set(TILE_DEFINE_FLAGS ${TILE_DEFINE_FLAGS} "_WIN32" "WIN32" "_WINDOWS"
         "_CRT_NONSTDC_NO_WARNINGS"
         "_CRT_SECURE_NO_WARNINGS"
  )
  if(NOT IS_DEBUG)
    set(TILE_OPTION_FLAGS ${TILE_OPTION_FLAGS} "/O2")
  endif()
elseif(UNIX)
  set(TILE_OPTION_FLAGS ${TILE_OPTION_FLAGS} "-fPIC" "-Wno-unused-result" )
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(TILE_OPTION_FLAGS ${TILE_OPTION_FLAGS} "-Wno-format-truncation")
  endif()
  if(NOT IS_DEBUG)
    set(TILE_OPTION_FLAGS ${TILE_OPTION_FLAGS} "-O2")
  endif()
endif()

if (LINUX)
  set(TILE_OPTION_FLAGS ${TILE_OPTION_FLAGS} "-m64" "-msse2")
endif()

#---------------------------------------
# Program Configuration
#---------------------------------------
if(MACOSX)
  set(TILE_LIBRARY_FLAGS PkgConfig::PNG ${ZLIB_LIBRARIES})
else()
  set(TILE_LIBRARY_FLAGS ${PNG_LIBRARIES})
endif()

set(TILE_DEFINE_FLAGS ${TILE_DEFINE_FLAGS} "ZLIB")

if(TLN_EXCLUDE_WINDOW)
  set(TILE_DEFINE_FLAGS ${TILE_DEFINE_FLAGS} "TLN_EXCLUDE_WINDOW")
endif()

if((HAVE_SDL2) AND (NOT TLN_EXCLUDE_WINDOW))
  set(TILE_INCLUDE_DIRS ${TILE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
  set(TILE_LIBRARY_FLAGS ${TILE_LIBRARY_FLAGS} ${SDL2_LIBRARIES})
  set(TILE_HEADER_FILES ${TILE_HEADER_FILES} "crt.h")
  set(TILE_SOURCE_FILES ${TILE_SOURCE_FILES} "crt.c")
endif()

#----------------------------------------------------------------------------------------------------------------------
# Target
#----------------------------------------------------------------------------------------------------------------------
# Add a shared library target
add_library(${PROJECT_NAME} SHARED ${PUBLIC_HEADER_FILES} ${TILE_HEADER_FILES} ${TILE_SOURCE_FILES})

# Make all non-exported symbols hidden by default and set version.
set_target_properties(${PROJECT_NAME} PROPERTIES 
					C_VISIBILITY_PRESET hidden
					VERSION ${PROJECT_VERSION}
					SOVERSION 2)

# Set target compile definitions.
target_compile_definitions(${PROJECT_NAME} PUBLIC ${TILE_DEFINE_FLAGS})

# Set target include directories and export the library's public header path to dependent targets.
target_include_directories(${PROJECT_NAME} PUBLIC ${TILE_INCLUDE_DIRS})

# Set compiler options.
target_compile_options(${PROJECT_NAME} PRIVATE ${TILE_OPTION_FLAGS})

# Set dependent link libraries.
target_link_libraries(${PROJECT_NAME} ${TILE_LIBRARY_FLAGS})

#----------------------------------------------------------------------------------------------------------------------
# Output
#----------------------------------------------------------------------------------------------------------------------
# List configuration properties of target during CMake generation.
set(CMAKE_PROPERTY_LIST SOURCE_DIR BINARY_DIR COMPILE_DEFINITIONS COMPILE_FLAGS COMPILE_OPTIONS
       INCLUDE_DIRECTORIES LINK_LIBRARIES LINK_FLAGS)

message(CHECK_START "Configuration for target ${PROJECT_NAME}")
message(CHECK_START ${PROJECT_DESCRIPTION})

foreach (property_type ${CMAKE_PROPERTY_LIST})
  get_property(property_value TARGET ${PROJECT_NAME} PROPERTY ${property_type} SET)
  if (property_value)
    get_target_property(property_value ${PROJECT_NAME} ${property_type})
    message (CHECK_START "${property_type} = ${property_value}")
  endif()
endforeach(property_type)

#----------------------------------------------------------------------------------------------------------------------

#----------------------------------------------------------------------------------------------------------------------
# Installation
#----------------------------------------------------------------------------------------------------------------------
if(NOT WIN32)
  set(target_lib ${PROJECT_NAME})
  configure_file(tilengine.pc.in tilengine.pc @ONLY)
  install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(FILES ${CMAKE_BINARY_DIR}/tilengine.pc
		DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
endif()

