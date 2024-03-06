# CMakeList.txt : CMake project for VirtualConsole, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("VirtualConsole" LANGUAGES CXX)

# Set program's application data path.
if(DEFINED USER_HOME_DIRECTORY_PATH AND NOT WIN32)
  if(MACOSX)
    set(PROJECT_DATA_DIR "${USER_HOME_DIRECTORY_PATH}/Library/Application Support/${PROJECT_NAME}")
  elseif(LINUX AND DEFINED HAS_XDG_CONFIG_PATH)
    set(PROJECT_DATA_DIR "${USER_HOME_DIRECTORY_PATH}/${PROJECT_NAME}")
  elseif(LINUX)
    set(PROJECT_DATA_DIR "${USER_HOME_DIRECTORY_PATH}/.config/${PROJECT_NAME}")
  endif()
endif()

# Create source files list.
set(SOURCE_FILES "VirtualConsole.cpp" "Common.h" "BasicMachine.cpp" "BasicMachine.h" "BasicScreen.h" "BasicScreen.cpp" "BasicMenu.h" "BasicMenu.cpp" "BasicGameSelect.h" "BasicGameSelect.cpp")

# Add executable.
if(MACOSX)
  # Get a list of all of the files in the asset data folder.
  file(GLOB_RECURSE RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Assets/*")

  # Define executable and add source files with asset files as resource files.
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILES} ${RESOURCE_FILES})
  set_target_properties(${PROJECT_NAME} PROPERTIES
                        BUNDLE True
                        MACOSX_BUNDLE_GUI_IDENTIFIER org.{$PROJECT_NAME}.gui
                        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
                        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
                        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
                        )

  # Individually set the file's path properties.
  foreach (FILE ${RESOURCE_FILES})
    # Get the relative path from the asset data folder to the particular file.
    file(RELATIVE_PATH NEW_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Assets" ${FILE})

    # Get the relative path to the file.
    get_filename_component(NEW_FILE_PATH ${NEW_FILE} DIRECTORY)

    # Set it's location inside the app package (under Resources).
    set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${NEW_FILE_PATH}")

    # Add the file to the 'Resources' folder group of the resulting bundle.
    source_group("Resources/${NEW_FILE_PATH}" FILES "${FILE}")
  endforeach ()
else()
  # Add source files to executable.
  add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

# Set target compile definitions.
target_compile_definitions(${PROJECT_NAME} PUBLIC "PROGRAM_NAME=\"${PROJECT_NAME}\"")

# Set required libraries to link against.
target_link_libraries(${PROJECT_NAME} VirtualMachine)

# Add PNG to Target Runtime DLLs on Windows.
if(WIN32)
  add_library(PNG SHARED IMPORTED)
endif()

# Copy required files to program's data folders.
if(NOT MACOSX)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
          ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/Assets" $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets)
endif()
if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
          ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/Settings" $<TARGET_FILE_DIR:${PROJECT_NAME}>/Settings)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
          ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/Cores" $<TARGET_FILE_DIR:${PROJECT_NAME}>/Cores)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
          ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/Content" $<TARGET_FILE_DIR:${PROJECT_NAME}>/Content)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
          ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/System" $<TARGET_FILE_DIR:${PROJECT_NAME}>/System)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
          ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/Saves" $<TARGET_FILE_DIR:${PROJECT_NAME}>/Saves)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
          ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/States" $<TARGET_FILE_DIR:${PROJECT_NAME}>/States)
elseif(DEFINED PROJECT_DATA_DIR)
  if(MACOSX)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND 
            ${CMAKE_COMMAND} -E make_directory "${PROJECT_DATA_DIR}/Settings")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
            ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/Settings" "${PROJECT_DATA_DIR}/Settings")
  elseif(LINUX)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND 
            ${CMAKE_COMMAND} -E make_directory "${PROJECT_DATA_DIR}/settings")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
            ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/Settings" "${PROJECT_DATA_DIR}/settings")
  endif()
endif()

# Copy Required DLLs to output folder on Windows.
if (CMAKE_IMPORT_LIBRARY_SUFFIX)
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
  )
endif ()