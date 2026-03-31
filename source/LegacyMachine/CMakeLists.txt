# CMakeList.txt : LegacyMachine CMake project file.

cmake_minimum_required (VERSION 3.23)

#----------------------------------------------------------------------------------------------------------------------
# Project
#----------------------------------------------------------------------------------------------------------------------

# Specify the C standard.
set(CMAKE_C_STANDARD 99)

project("LegacyMachine" VERSION 0.0.7 LANGUAGES C)

configure_file(libraryversion.h.in "${PROJECT_SOURCE_DIR}/LibraryVersion.h")

#----------------------------------------------------------------------------------------------------------------------
# Configuration
#----------------------------------------------------------------------------------------------------------------------

set(RETRO_INCLUDE_DIRS 
		"${PROJECT_SOURCE_DIR}/include" 
		${LIBRETRO_INCLUDE_DIRS}
		${DEPENDENCIES_DIR}
)

# List all public library headers
set(PUBLIC_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/LegacyMachine.h")

set(RETRO_HEADER_FILES
		"Common/Common.h"
		"Logging.h"	
		"CoreLibrary.h"
		"MainEngine.h"
		"SystemManager.h"
		"SettingsManager.h"
		"OptionsManager.h"
		"StateList.h"
		"TaskHandler.h"
		"Platform/PlatformManager.h"
		"Window/WindowManager.h"
		"Video/VideoManager.h"
		"Audio/AudioManager.h"
		"Input/InputManager.h"
		"Video/CRTEffect.h"
		"Video/Blitters/Blitters.h"
		"${LIBRETRO_INCLUDE_DIR}/libretro.h"
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
		"${LIBRETRO_INCLUDE_DIR}/retro_types.h"
		"${LIBRETRO_INCLUDE_DIR}/array/rhmap.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/fopen_utf8.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/getopt.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/posix_string.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/strcasestr.h"
		"${LIBRETRO_INCLUDE_DIR}/compat/strl.h"
		"${LIBRETRO_INCLUDE_DIR}/dynamic/dylib.h"
		"${LIBRETRO_INCLUDE_DIR}/encodings/utf.h"
		"${LIBRETRO_INCLUDE_DIR}/file/config_file.h"
		"${LIBRETRO_INCLUDE_DIR}/file/file_path.h"
		"${LIBRETRO_INCLUDE_DIR}/features/features_cpu.h"
		"${LIBRETRO_INCLUDE_DIR}/gfx/scaler/pixconv.h"
		"${LIBRETRO_INCLUDE_DIR}/lists/dir_list.h"
		"${LIBRETRO_INCLUDE_DIR}/lists/linked_list.h"
		"${LIBRETRO_INCLUDE_DIR}/lists/nested_list.h"
		"${LIBRETRO_INCLUDE_DIR}/lists/string_list.h"
		"${LIBRETRO_INCLUDE_DIR}/queues/message_queue.h"
		"${LIBRETRO_INCLUDE_DIR}/queues/task_queue.h"
		"${LIBRETRO_INCLUDE_DIR}/streams/file_stream.h"
		"${LIBRETRO_INCLUDE_DIR}/streams/interface_stream.h"
		"${LIBRETRO_INCLUDE_DIR}/streams/memory_stream.h"
		"${LIBRETRO_INCLUDE_DIR}/string/stdstring.h"
		"${LIBRETRO_INCLUDE_DIR}/time/rtime.h"
		"${LIBRETRO_INCLUDE_DIR}/vfs/vfs_implementation.h"
)

set(RETRO_SOURCE_FILES 
		"LegacyMachine.c"
		"SystemManager.c"
		"SettingsManager.c"
		"OptionsManager.c"
		"StateList.c"
		"TaskHandler.c"
		"Platform/PlatformManager.c"
		"Window/WindowManager.c"
		"Video/VideoManager.c"
		"Audio/AudioManager.c"
		"Input/InputManager.c"
		"Video/CRTEffect.c"
		"Video/Blitters/Blitters.c"
		"Platform.c"
		"System.c"
		"Window.c"
		"Video.c"
		"Input.c"
		"${LIBRETRO_SOURCE_DIR}/compat/fopen_utf8.c"
		"${LIBRETRO_SOURCE_DIR}/compat/compat_posix_string.c"
		"${LIBRETRO_SOURCE_DIR}/dynamic/dylib.c"
		"${LIBRETRO_SOURCE_DIR}/encodings/encoding_utf.c"
		"${LIBRETRO_SOURCE_DIR}/features/features_cpu.c"
		"${LIBRETRO_SOURCE_DIR}/file/config_file.c"
		"${LIBRETRO_SOURCE_DIR}/file/file_path.c"
		"${LIBRETRO_SOURCE_DIR}/file/file_path_io.c"
		"${LIBRETRO_SOURCE_DIR}/file/retro_dirent.c"
		"${LIBRETRO_SOURCE_DIR}/gfx/scaler/pixconv.c"
		"${LIBRETRO_SOURCE_DIR}/lists/dir_list.c"
		"${LIBRETRO_SOURCE_DIR}/lists/linked_list.c"
		"${LIBRETRO_SOURCE_DIR}/lists/nested_list.c"
		"${LIBRETRO_SOURCE_DIR}/lists/string_list.c"
		"${LIBRETRO_SOURCE_DIR}/queues/message_queue.c"
		"${LIBRETRO_SOURCE_DIR}/queues/task_queue.c"
		"${LIBRETRO_SOURCE_DIR}/streams/file_stream.c"
		"${LIBRETRO_SOURCE_DIR}/streams/interface_stream.c"
		"${LIBRETRO_SOURCE_DIR}/streams/memory_stream.c"
		"${LIBRETRO_SOURCE_DIR}/string/stdstring.c"
		"${LIBRETRO_SOURCE_DIR}/time/rtime.c"
		"${LIBRETRO_SOURCE_DIR}/vfs/vfs_implementation.c"
)

set(RETRO_DEFINE_FLAGS "")
set(RETRO_OPTION_FLAGS "")
set(RETRO_LIBRARY_FLAGS "")
set(RETRO_LINKER_FLAGS "LINKER:")
set(HAVE_MENU ON)

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
elseif(UNIX)
  if(MACOSX)
    set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${COCOA_LIBRARY})
	set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "Platform/Drivers/MacOS_PlatformDriver.m")
  else()
    set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "Platform/Drivers/Unix_PlatformDriver.c")
  endif()
  set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-fPIC" "-Wno-unused-result")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-Wno-format-truncation")
  endif()
endif()

if (LINUX)
  set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-m64" "-msse2")
  set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} "-lm")
  if(IS_DEBUG)
    set(RETRO_OPTION_FLAGS ${RETRO_OPTION_FLAGS} "-fstack-usage")
  endif()
endif()

set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} 
		"LIB_EXPORTS"
		"LEGACY_MACHINE_EXPORTS"
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
  set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${CMAKE_THREAD_LIBS_INIT})
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
# Multimedia Configuration
#---------------------------------------
if(HAVE_SDL)
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_SDL")
  if(HAVE_SDL3)
	set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_SDL3")
	set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} ${SDL3_INCLUDE_DIRS})
	set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${SDL3_LIBRARIES})
  elseif(HAVE_SDL2)
	set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_SDL2")
	set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
	set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${SDL2_LIBRARIES})
  endif()
  set(RETRO_HEADER_FILES ${RETRO_HEADER_FILES} "Video/Filters/SDL_CRTFilter.h"
		"Common/SDL_Common.h"
  )
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES}
		"Common/SDL_Common.c"
		"Window/Drivers/SDL_WindowDriver.c"
		"Video/Drivers/SDL_VideoDriver.c"
		"Audio/Drivers/SDL_AudioDriver.c"
		"Input/Drivers/SDL_InputDriver.c"
		"Video/Filters/SDL_CRTFilter.c"
  )
endif()


#---------------------------------------
# Compression/Archive Configuration
#---------------------------------------
if(HAVE_COMPRESSION)
  set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_COMPRESSION")
  set(RETRO_HEADER_FILES ${RETRO_HEADER_FILES} "${LIBRETRO_INCLUDE_DIR}/encodings/crc32.h"
		"${LIBRETRO_INCLUDE_DIR}/file/archive_file.h"
		"${LIBRETRO_INCLUDE_DIR}/streams/trans_stream.h"
  )
  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/encodings/encoding_crc32.c"
		"${LIBRETRO_SOURCE_DIR}/file/archive_file.c"
		"${LIBRETRO_SOURCE_DIR}/streams/trans_stream.c"
		"${LIBRETRO_SOURCE_DIR}/streams/trans_stream_pipe.c"
  )

  if(HAVE_ZLIB)
    set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_ZLIB")
    set(RETRO_LIBRARY_FLAGS ${RETRO_LIBRARY_FLAGS} ${ZLIB_LIBRARIES})
    set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
	set(RETRO_HEADER_FILES ${RETRO_HEADER_FILES} "${LIBRETRO_INCLUDE_DIR}/streams/rzip_stream.h")
	set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/file/archive_file_zlib.c"
         "${LIBRETRO_SOURCE_DIR}/streams/trans_stream_zlib.c"
         "${LIBRETRO_SOURCE_DIR}/streams/rzip_stream.c"
    )
  endif()

  if(HAVE_7ZIP)
    set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_7ZIP" "_7ZIP_ST")
	set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} "${SEVENZIP_SOURCE_DIR}")
    set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/file/archive_file_7z.c"
		 "${SEVENZIP_SOURCE_DIR}/7zArcIn.c"
         "${SEVENZIP_SOURCE_DIR}/7zBuf.c"
         "${SEVENZIP_SOURCE_DIR}/7zCrc.c"
         "${SEVENZIP_SOURCE_DIR}/7zCrcOpt.c"
         "${SEVENZIP_SOURCE_DIR}/7zDec.c"
         "${SEVENZIP_SOURCE_DIR}/CpuArch.c"
         "${SEVENZIP_SOURCE_DIR}/Delta.c"
         "${SEVENZIP_SOURCE_DIR}/LzFind.c"
         "${SEVENZIP_SOURCE_DIR}/LzmaDec.c"
         "${SEVENZIP_SOURCE_DIR}/Lzma2Dec.c"
         "${SEVENZIP_SOURCE_DIR}/LzmaEnc.c"
         "${SEVENZIP_SOURCE_DIR}/Bra.c"
         "${SEVENZIP_SOURCE_DIR}/Bra86.c"
         "${SEVENZIP_SOURCE_DIR}/BraIA64.c"
         "${SEVENZIP_SOURCE_DIR}/Bcj2.c"
         "${SEVENZIP_SOURCE_DIR}/7zFile.c"
         "${SEVENZIP_SOURCE_DIR}/7zStream.c"
    )
  endif()

  if(HAVE_CHD)
    set(RETRO_DEFINE_FLAGS ${RETRO_DEFINE_FLAGS} "HAVE_CHD" "WANT_SUBCODE" "WANT_RAW_DATA_SECTOR")
    set(RETRO_INCLUDE_DIRS ${RETRO_INCLUDE_DIRS} "${LIBRETRO_SOURCE_DIR}/formats/libchdr")
    set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/formats/libchdr/libchdr_bitstream.c"
           "${LIBRETRO_SOURCE_DIR}/formats/libchdr/libchdr_cdrom.c"
           "${LIBRETRO_SOURCE_DIR}/formats/libchdr/libchdr_chd.c"
           "${LIBRETRO_SOURCE_DIR}/formats/libchdr/libchdr_huffman.c"
           "${LIBRETRO_SOURCE_DIR}/streams/chd_stream.c"
           "${LIBRETRO_SOURCE_DIR}/formats/libchdr/libchdr_zlib.c"
    )

	if(HAVE_7ZIP)
	  set(RETRO_SOURCE_FILES ${RETRO_SOURCE_FILES} "${LIBRETRO_SOURCE_DIR}/formats/libchdr/libchdr_lzma.c")
	endif()
  endif()
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