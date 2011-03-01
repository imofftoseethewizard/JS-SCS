#/**********************************************************\ 
# 
# Auto-generated CMakeLists.txt for the HostServices project
#
#\**********************************************************/

# Written to work with cmake 2.6
cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)

include_directories("/usr/include/atkmm-1.6/")
include_directories("/usr/include/cairomm-1.0/")
include_directories("/usr/include/gdkmm-2.4/")
include_directories("/usr/include/giomm-2.4/")
include_directories("/usr/include/glibmm-2.4/")
include_directories("/usr/include/gtkmm-2.4/")
include_directories("/usr/include/pangomm-1.4/")
include_directories("/usr/include/sigc++-2.0/")

include_directories("/usr/lib/atkmm-1.6/include/")
include_directories("/usr/lib/cairomm-1.0/include/")
include_directories("/usr/lib/gdkmm-2.4/include/")
include_directories("/usr/lib/giomm-2.4/include/")
include_directories("/usr/lib/glibmm-2.4/include/")
include_directories("/usr/lib/gtkmm-2.4/include/")
include_directories("/usr/lib/pangomm-1.4/include/")
include_directories("/usr/lib/sigc++-2.0/include/")


Project(${PLUGIN_NAME})

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.cpp
    [^.]*.h
    [^.]*.cmake
    )

include_directories(${PLUGIN_INCLUDE_DIRS})

# Generated files are stored in ${GENERATED} by the project configuration
SET_SOURCE_FILES_PROPERTIES(
    ${GENERATED}
    PROPERTIES
        GENERATED 1
    )

SOURCE_GROUP(Generated FILES
    ${GENERATED}
    )

SET( SOURCES
    ${GENERAL}
    ${GENERATED}
    )

# This will include Win/projectDef.cmake, Linux/projectDef.cmake, etc
include_platform()
