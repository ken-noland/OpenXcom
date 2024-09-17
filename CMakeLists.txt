cmake_minimum_required ( VERSION 3.1 )

project ( OpenXcom )

set ( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" )
include(BuildType)
include(GNUInstallDirs)

# For yaml-cpp
set (CMAKE_CXX_STANDARD 20)

option ( DEV_BUILD "Development Build. Disable this for release builds" ON )
option ( DUMP_CORE "Disables exception and segfault handling." OFF )
option ( BUILD_PACKAGE "Prepares build for creation of a package with CPack" ON )
set ( TARGET_PLATFORM CACHE STRING "Target platform to include in the package name (win32, etc)" )
option ( EMBED_ASSETS "Embed common and standard into the executable" OFF )
option ( FATAL_WARNING "Treat warnings as errors" OFF )
option ( ENABLE_CLANG_ANALYSIS "When building with clang, enable the static analyzer" OFF )
option ( CHECK_CCACHE "Check if ccache is installed and use it" OFF )
set ( MSVC_WARNING_LEVEL 3 CACHE STRING "Visual Studio warning levels" )
option ( FORCE_INSTALL_DATA_TO_BIN "Force installation of data to binary directory" OFF )
set ( DATADIR "" CACHE STRING "Where to place datafiles" )

if ( CHECK_CCACHE )
  find_program( CCACHE_PROGRAM ccache )
  if( NOT CCACHE_PROGRAM )
    message ( FATAL_ERROR "CCACHE requested but not found on the system." )
  else ()
    set_property( GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}" )
    set_property( GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}" )
    message ( STATUS "found CCACHE (${CCACHE_PROGRAM})" )
  endif()
endif ()

if ( WIN32 )
  set ( default_deps_dir "${CMAKE_SOURCE_DIR}/deps" )
endif ()

if ( APPLE )
  option ( CREATE_BUNDLE "Create a Mac OS application bundle" ON )
  if ( NOT EXISTS "${MACOS_SDLMAIN_M_PATH}" )
    message ( FATAL_ERROR "On Mac OS, SDLMain.m is required. Please set the MACOS_SDLMAIN_M_PATH variable" )
  endif ()
endif ()

# C++ version check
if ( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
  if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
  else()
    message( FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("/std:c++20" COMPILER_SUPPORTS_CXX20)
  if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /Zc:__cplusplus")
  else()
    message( FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use new VS C++ compiler.")
  endif()
endif()

set ( DEPS_DIR "${default_deps_dir}" CACHE STRING "Dependencies directory" )

# Read version number
set ( file "${CMAKE_SOURCE_DIR}/src/version.h" )
file ( READ ${file} lines )
string ( REGEX MATCH ".*OPENXCOM_VERSION_LONG \"([0-9]+)\\.([0-9]+)\\.([0-9]+)" version_line "${lines}" )
set ( CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1} )
set ( CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2} )
set ( CPACK_PACKAGE_VERSION_PATCH "${CMAKE_MATCH_3}" )

find_package ( Git )
if ( GIT_FOUND )
  execute_process ( COMMAND ${GIT_EXECUTABLE} describe --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE git_describe_out
    ERROR_VARIABLE git_describe_error
    RESULT_VARIABLE git_describe_result
    )
  string ( REGEX MATCH "([a-z|0-9|.]*)-([0-9]*)-g([a-z|0-9]*)([-|a-z]*)" git_commit "${git_describe_out}" )
  set ( git_tag ${CMAKE_MATCH_1} )
  set ( git_nb_commit ${CMAKE_MATCH_2} )
  set ( git_commit ${CMAKE_MATCH_3} )
  set ( git_dirty ${CMAKE_MATCH_4} )
  execute_process ( COMMAND ${GIT_EXECUTABLE} show -s --format=%ci
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE git_show_out
    ERROR_VARIABLE git_show_error
    RESULT_VARIABLE git_show_result
  )
  string ( REGEX MATCH "[0-9-]+" git_date "${git_show_out}" )
  set ( GIT_STATE "${git_commit}-${git_date}${git_dirty}" )
else ()
  message( WARNING "Git not found.")
endif()

set ( OPENXCOM_VERSION_GIT "${GIT_STATE}" CACHE STRING "Git version string (after x.x.x)" )

configure_file("${CMAKE_SOURCE_DIR}/src/git_version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/git_version.h" )
include_directories ( "${CMAKE_CURRENT_BINARY_DIR}" )

if ( BUILD_PACKAGE )
  set ( CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${GIT_STATE}" )
  set ( CPACK_SOURCE_PACKAGE_FILE_NAME "Extended-${CPACK_PACKAGE_VERSION}-src" )
  set ( CPACK_PACKAGE_FILE_NAME "Extended-${CPACK_PACKAGE_VERSION}-unknown" ) # TODO: determine package suffix (osrelease+arch)

  if ( NOT CPACK_GENERATOR )
    set ( CPACK_GENERATOR "TXZ" )
  endif ()
  if ( NOT CPACK_SOURCE_GENERATOR )
    set ( CPACK_SOURCE_GENERATOR "TXZ" )
  endif ()

  set ( CPACK_PACKAGE_VENDOR "The OpenXcom project" )
  set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open-source clone of UFO: Enemy Unknown" )
  set ( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/cmake/modules/Description.txt" )
  set ( CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md" )
  set ( CPACK_PACKAGE_CONTACT "The OpenXcom project (http://www.openxcom.org)" )
  # DMG-specific -- sets CPACK_GENERATOR to DragNDrop itself
  if (APPLE)
    set ( TARGET_PLATFORM "macos" )
    set ( CPACK_BUNDLE_NAME "OpenXcom Extended" )
    set ( CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/res/mac/AppIcon.icns" )
    set ( CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/openxcom.app/Contents/Info.plist" )
    set ( CPACK_DMG_VOLUME_NAME "OpenXcom Extended" )
    set ( CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/res/openxcom_block.png" )
    set ( CPACK_GENERATOR "DragNDrop")
  else()
    set ( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt" )
  endif()

  string ( LENGTH "${TARGET_PLATFORM}" has_target )
  if ( ${has_target} GREATER 0 )
    set ( CPACK_PACKAGE_FILE_NAME "Extended-${CPACK_PACKAGE_VERSION}-${TARGET_PLATFORM}" )
  else ()
    set ( CPACK_PACKAGE_FILE_NAME "Extended-${CPACK_PACKAGE_VERSION}-unknown" )
    message ( WARNING "Missing OPENXCOM_TARGET for package naming. Please set it." )
  endif()

  # DEB-specific -- set -DCPACK_GENERATOR=DEB
  if ( "${CPACK_GENERATOR}" MATCHES "^(.*;)?DEB(;.*)?$" )
    set ( CPACK_DEBIAN_PACKAGE_MAINTAINER "The OpenXcom project" )
    set ( CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://openxcom.org/" )
    set ( CPACK_DEBIAN_PACKAGE_SECTION "games" )
    set ( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )
    set ( CPACK_DEBIAN_COMPRESSION_TYPE "xz" )
    if ( "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" )
      set ( CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64" )
    elseif ( "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686" )
      set ( CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386" )
    endif ()
    if ( NOT CPACK_DEBIAN_PACKAGE_DEPENDS )
      message ( STATUS "CPACK_DEBIAN_PACKAGE_DEPENDS not set - DEB package will have no dependencies" )
    endif ()
  endif ()
  # RPM-specific -- set -DCPACK_GENERATOR=RPM
  if ( "${CPACK_GENERATOR}" MATCHES "^(.*;)?RPM(;.*)?$" )
    set ( CPACK_RPM_PACKAGE_LICENSE "GPLv3" )
    set ( CPACK_RPM_PACKAGE_GROUP "Amusements/Games" )
    set ( CPACK_RPM_PACKAGE_URL "https://openxcom.org/" )
    set ( CPACK_RPM_COMPRESSION_TYPE "xz" )
    if ( NOT CPACK_RPM_PACKAGE_REQUIRES )
      message ( STATUS "CPACK_RPM_PACKAGE_REQUIRES not set - RPM package will have no dependencies" )
    endif ()
  endif ()
  # NSIS-specific -- set -DCPACK_GENERATOR=NSIS
  set ( CPACK_NSIS_MODIFY_PATH OFF )
  set ( CPACK_NSIS_DISPLAY_NAME "OpenXcom Extended" )
  include ( CPack )
  message ( STATUS "OpenXcom version: ${CPACK_PACKAGE_VERSION}")
endif()

if ( NOT WIN32 OR NOT CPACK_GENERATOR STREQUAL "7Z" ) # FIXME: that's to exclude DEB and RPM, but hacky.
  install(FILES "${CMAKE_SOURCE_DIR}/res/linux/openxcom.desktop"
    DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications")
  install(FILES "${CMAKE_SOURCE_DIR}/res/linux/icons/openxcom_48x48.png"
    DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/48x48/apps" RENAME openxcom.png)
  install(FILES "${CMAKE_SOURCE_DIR}/res/linux/icons/openxcom_128x128.png"
    DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/128x128/apps" RENAME openxcom.png)
  install(FILES "${CMAKE_SOURCE_DIR}/res/linux/icons/openxcom.svg"
    DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/scalable/apps")
endif ()

add_subdirectory ( docs )
add_subdirectory ( src )