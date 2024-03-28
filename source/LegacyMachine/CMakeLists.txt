# CMakeList.txt : LegacyMachine CMake project file.

cmake_minimum_required (VERSION 3.23)

#----------------------------------------------------------------------------------------------------------------------
# Project
#----------------------------------------------------------------------------------------------------------------------
project("LegacyMachine" VERSION 0.0.9 LANGUAGES C)

#----------------------------------------------------------------------------------------------------------------------
# Configuration
#----------------------------------------------------------------------------------------------------------------------

set(RETRO_INCLUDE_DIRS 
		"${PROJECT_SOURCE_DIR}/include" 
		${LIBRETRO_INCLUDE_DIRS} 
)

# List all public library headers
set(PUBLIC_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/LegacyMachine.h")

set(RETRO_HEADER_FILES
		"Common/Common.h"
		"Logging.h"	
		"CoreLibrary.h"
		"MainEngine.h"
		"SettingsManager.h"
		"Platform/PlatformDriver.h"
		"Window/WindowDriver.h"
		"Video/VideoDriver.h"
		"Audio/AudioDriver.h"
		"Input/InputDriver.h"
		"Video/CRTFilter.h"
		"SystemManager.h"		
		"${LIBRETRO_INCLUDE_DIR}/libretro.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_library.h"
		"${LIBRETRO_INCLUDE_DIR}/boolean.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_assert.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_common.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_common_api.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_dirent.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_endianness.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_environment.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_inline.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_math.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_miscellaneous.h"
		"${LIBRETRO_INCLUDE_DIR}/retro_timers.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/fopen_utf8.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/getopt.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/posix_string.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/strcasestr.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/strl.h"
		"${LIBRETRO_INCLUDE_DIR}/dynamic/dylib.h"
		"${LIBRETRO_INCLUDE_DIR}/encodings/utf.h"
		"${LIBRETRO_INCLUDE_DIR}/file/file_path.h"
		"${LIBRETRO_INCLUDE_DIR}/features/features_cpu.h"
		"${LIBRETRO_INCLUDE_DIR}/lists/linked_list.h"
		"${LIBRETRO_INCLUDE_DIR}/streams/file_stream.h"
		"${LIBRETRO_INCLUDE_DIR}/string/stdstring.h"
		"${LIBRETRO_INCLUDE_DIR}/time/rtime.h"
		"${LIBRETRO_INCLUDE_DIR}/vfs/vfs_implementation.h"
)

set(RETRO_SOURCE_FILES 
		"LegacyMachine.c"
		"SettingsManager.c"
		"Platform/PlatformDriver.c"
		"Window/WindowDriver.c"
		"Video/VideoDriver.c"
		"Audio/AudioDriver.c"
		"Input/InputDriver.c"
		"Video/CRTFilter.c"
		"Video/Filters/RFBlur.c"
		"SystemManager.c"
		"Window.c"
		"${LIBRETRO_SOURCE_DIR}/compat/fopen_utf8.c"
		"${LIBRETRO_SOURCE_DIR}/compat/compat_posix_string.c"
		"${LIBRETRO_SOURCE_DIR}/dynamic/dylib.c"
		"${LIBRETRO_SOURCE_DIR}/encodings/encoding_utf.c"
		"${LIBRETRO_SOURCE_DIR}/features/features_cpu.c"
		"${LIBRETRO_SOURCE_DIR}/file/file_path.c"
		"${LIBRETRO_SOURCE_DIR}/file/file_path_io.c"
		"${LIBRETRO_SOURCE_DIR}/lists/linked_list.c"
		"${LIBRETRO_SOURCE_DIR}/streams/file_stream.c"
		"${LIBRETRO_SOURCE_DIR}/string/stdstring.c"
		"${LIBRETRO_SOURCE_DIR}/time/rtime.c"
		"${LIBRETRO_SOURCE_DIR}/vfs/vfs_implementation.c"
)

set(RETRO_DEFINE_FLAGS "")
set(RETRO_OPTION_FLAGS "")
set(RETRO_LIBRARY_FLAGS "")
set(RETRO_LINKER_FLAGS "LINKER:")

#---------------------------------------
# Build Configuration
#---------------------------------------
if(IS_DEBUG)
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "_DEBUG" "DEBUG")
else()
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "NDEBUG")
endif()

#---------------------------------------
# Platform Configuration
#---------------------------------------
if(WIN32)
  if(MSVC_VERSION LESS 1600)
    set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} "${LIBRETRO_INCLUDE_DIR}/compat/msvc")
  endif()
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "_WIN32" "WIN32" "_WINDOWS"
         "_CRT_NONSTDC_NO_WARNINGS"
         "_CRT_SECURE_NO_WARNINGS"
  )
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "Platform/Drivers/Win32_PlatformDriver.c")
  if(NOT IS_DEBUG)
    set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "/O2")
  endif()
elseif(UNIX)
  if(MACOSX)
    set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${COCOA_LIBRARY})
	set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "Platform/Drivers/MacOS_PlatformDriver.m")
  else()
    set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "Platform/Drivers/Unix_PlatformDriver.c")
  endif()
  set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-fPIC" "-Wno-unused-result")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(TILE_OPTION_FLAGS ${TILE_OPTION_FLAGS} "-Wno-format-truncation")
  endif()
  if(NOT IS_DEBUG)
    set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-O2")
  endif()
endif()

if (LINUX)
  set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-m64" "-msse2")
  if(IS_DEBUG)
    set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-fstack-usage")
  endif()
endif()

set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} 
		"LIB_EXPORTS"
		"RETRO_LIB_EXPORTS"
		"HAVE_DYNAMIC"
		${LIBRETRO_COMMON_DEFINE_FLAGS}
)

if(HAVE_MENU AND HAVE_PNG)
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_MENU")
  set(RETRO_HEADER_FILES ${RETRO_HEADER_FILES} "Menu/MenuManager.h")
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "Menu/MenuManager.c")
  set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} ${TILENGINE_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
  set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} Tilengine)
  if(MACOSX)
    set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} PkgConfig::PNG ${ZLIB_LIBRARIES})
  else()
    set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${PNG_LIBRARIES})
  endif()  
endif()

if(HAVE_THREADS)
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_THREADS")
  set(RETRO_HEADER_FILES ${RETRO_HEADER_FILES} "${LIBRETRO_INCLUDE_DIR}/rthreads/rthreads.h")
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/rthreads/rthreads.c")
endif()

if(HAVE_NEON)
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_NEON")
  set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-mfpu=neon" "-marm")
endif()

if(NOT HAVE_GETOPT_LONG)
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/compat/compat_getopt.c")
else()
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_GETOPT_LONG")
endif()

if(NOT HAVE_STRCASESTR)
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/compat/compat_strcasestr.c")
else()
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_STRCASESTR")
endif()

if(NOT HAVE_STRL)
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/compat/compat_strl.c")
else()
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_STRL")
endif()

#---------------------------------------
# Video Configuration
#---------------------------------------
if(HAVE_OPENGL)
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_OPENGL")
  set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} ${GLAD_INCLUDE_DIR})
  #set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${OPENGL_LIBRARIES})
  set(RETRO_HEADER_FILES ${RETRO_HEADER_FILES} "${GLAD_INCLUDE_DIR}/glad/glad.h"
		"${GLAD_INCLUDE_DIR}/KHR/khrplatform.h"
		"Common/GL_Common.h"
		"Video/Shared/GL_Shared.h"
  )
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${GLAD_SOURCE_DIR}/glad.c"
		"Common/GL_Common.c"
		"Video/Shared/GL_Shared.c"
  )
endif()

#---------------------------------------
# Multimedia Configuration
#---------------------------------------
if(HAVE_SDL2)
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_SDL2")
  set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
  set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${SDL2_LIBRARIES})
  set(RETRO_HEADER_FILES ${RETRO_HEADER_FILES} "Video/Filters/SDL2_CRTFilter.h"
		"Common/SDL2_Common.h"
  )
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES}
		"Common/SDL2_Common.c"
		"Window/Drivers/SDL2_WindowDriver.c"
		"Video/Drivers/SDL2_VideoDriver.c"
		"Audio/Drivers/SDL2_AudioDriver.c"
		"Input/Drivers/SDL2_InputDriver.c"
		"Video/Filters/SDL2_CRTFilter.c"
  )
endif()

#----------------------------------------------------------------------------------------------------------------------
# Target
#----------------------------------------------------------------------------------------------------------------------
# Add a shared library target
add_library(${PROJECT_NAME} SHARED ${PUBLIC_HEADER_FILES} ${RETRO_HEADER_FILES} ${RETRO_SOURCE_FILES})

# Make all non-exported symbols hidden by default and set version.
set_target_properties(${PROJECT_NAME} PROPERTIES 
					C_VISIBILITY_PRESET hidden
					VERSION ${PROJECT_VERSION}
					SOVERSION 0)

# Set include directories.
target_include_directories(${PROJECT_NAME} PUBLIC ${RETRO_INCLUDE_DIRS})

# Set required libraries to link against.
target_link_libraries(${PROJECT_NAME} ${RETRO_LIBRARY_FLAGS})

# Set preprocessor definitions.
target_compile_definitions(${PROJECT_NAME} PRIVATE ${RETRO_DEFINE_FLAGS})

# Set compiler options.
target_compile_options(${PROJECT_NAME} PRIVATE ${RETRO_OPTION_FLAGS})

# Set linker options.
target_link_options(${PROJECT_NAME} PUBLIC ${RETRO_LINKER_FLAGS})

#----------------------------------------------------------------------------------------------------------------------
# Output
#----------------------------------------------------------------------------------------------------------------------
# List configuration properties of target during CMake generation.
set(CMAKE_PROPERTY_LIST SOURCE_DIR BINARY_DIR COMPILE_DEFINITIONS COMPILE_FLAGS COMPILE_OPTIONS
       INCLUDE_DIRECTORIES LINK_LIBRARIES LINK_FLAGS)

message(CHECK_START "Configuration for target ${PROJECT_NAME} ${CMAKE_BUILD_TYPE}")

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
  configure_file(legacymachine.pc.in legacymachine.pc @ONLY)
  install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(FILES ${CMAKE_BINARY_DIR}/legacymachine.pc
		DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
endif()