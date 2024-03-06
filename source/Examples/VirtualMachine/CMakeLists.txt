# CMakeList.txt : CMake project for VirtualMachine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("VirtualMachine" LANGUAGES CXX)

#----------------------------------------------------------------------------------------------------------------------
# Configuration
#----------------------------------------------------------------------------------------------------------------------
set(VM_INCLUDE_DIRS 
		"${PROJECT_SOURCE_DIR}/include"
)

# List all public library headers
set(PUBLIC_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/VirtualMachine.h"
		"${PROJECT_SOURCE_DIR}/include/IMachine.h"
)

# List all required library headers
set(VM_HEADER_FILES 
		"MenuOption.h"
		"ContentOption.h"
)

# List all required library sources
set(VM_SOURCE_FILES 
		"VirtualMachine.cpp"
		"MenuOption.cpp"
		"ContentOption.cpp"
)

set(VM_DEFINE_FLAGS "")
set(VM_OPTION_FLAGS "")

#---------------------------------------
# Build Configuration
#---------------------------------------
if(IS_DEBUG)
  set(VM_DEFINE_FLAGS ${VM_DEFINE_FLAGS} "_DEBUG" "DEBUG")
else()
  set(VM_DEFINE_FLAGS ${VM_DEFINE_FLAGS} "NDEBUG")
endif()

#---------------------------------------
# Platform Configuration
#---------------------------------------
if(WIN32)
  set(VM_DEFINE_FLAGS ${VM_DEFINE_FLAGS} "_WIN32" "WIN32" "_WINDOWS"
         "_CRT_NONSTDC_NO_WARNINGS"
         "_CRT_SECURE_NO_WARNINGS"
  )
else()
  set(VM_OPTION_FLAGS "-Wno-unused-result")
endif()

set(VM_DEFINE_FLAGS ${VM_DEFINE_FLAGS} ${LIBRETRO_COMMON_DEFINE_FLAGS} "HAVE_MENU")

#----------------------------------------------------------------------------------------------------------------------
# Target
#----------------------------------------------------------------------------------------------------------------------
# Add source to this library.
add_library(${PROJECT_NAME} STATIC ${PUBLIC_HEADER_FILES} ${VM_HEADER_FILES} ${VM_SOURCE_FILES})

# Add include directory for this library.
target_include_directories(${PROJECT_NAME} PUBLIC ${VM_INCLUDE_DIRS})

# Link required external libraries to this library.
target_link_libraries(${PROJECT_NAME} LegacyMachine Tilengine)

# Set preprocessor definitions.
target_compile_definitions(${PROJECT_NAME} PRIVATE ${VM_DEFINE_FLAGS})

# Set compiler options.
target_compile_options(${PROJECT_NAME} PRIVATE ${VM_OPTION_FLAGS})

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