#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  osm-history-splitter
#
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


#-----------------------------------------------------------------------------
#
#  Project version
#
#-----------------------------------------------------------------------------

project(osm-history-importer)

set(OHS_VERSION_MAJOR 0)
set(OHS_VERSION_MINOR 0)
set(OHS_VERSION_PATCH 1)

set(OHS_VERSION
    ${OHS_VERSION_MAJOR}.${OHS_VERSION_MINOR}.${OHS_VERSION_PATCH})

set(AUTHOR "Peter Körner <osm@mazdermind.de>")


#-----------------------------------------------------------------------------
#
#  Find external dependencies
#
#-----------------------------------------------------------------------------

find_package(GEOS)
find_package(Osmium COMPONENTS io proj)
find_package(PostgreSQL REQUIRED)
include_directories(${OSMIUM_INCLUDE_DIRS})
include_directories(${PostgreSQL_INCLUDE_DIR})
include_directories(${GEOS_INCLUDE_DIR})

#-----------------------------------------------------------------------------
#
#  Decide which C++ version to use (Minimum/default: C++11).
#
#-----------------------------------------------------------------------------
if(NOT MSVC)
    if(NOT USE_CPP_VERSION)
        set(USE_CPP_VERSION c++11)
    endif()
    message(STATUS "Use C++ version: ${USE_CPP_VERSION}")
    # following only available from cmake 2.8.12:
    #   add_compile_options(-std=${USE_CPP_VERSION})
    # so using this instead:
    add_definitions(-std=${USE_CPP_VERSION})
endif()


#-----------------------------------------------------------------------------
#
#  Compiler and Linker flags
#
#-----------------------------------------------------------------------------
set(USUAL_COMPILE_OPTIONS "-O3 -g")

set(CMAKE_CXX_FLAGS_DEV "${USUAL_COMPILE_OPTIONS}"
    CACHE STRING "Flags used by the compiler during developer builds."
    FORCE)

set(CMAKE_EXE_LINKER_FLAGS_DEV ""
    CACHE STRING "Flags used by the linker during developer builds."
    FORCE)
mark_as_advanced(
    CMAKE_CXX_FLAGS_DEV
    CMAKE_EXE_LINKER_FLAGS_DEV
)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${USUAL_COMPILE_OPTIONS}"
    CACHE STRING "Flags used by the compiler during RELWITHDEBINFO builds."
    FORCE)


#-----------------------------------------------------------------------------
#
#  Build Type
#
#-----------------------------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES "Debug Release RelWithDebInfo MinSizeRel Dev")

# In 'Dev' mode: compile with very strict warnings and turn them into errors.
if(CMAKE_BUILD_TYPE STREQUAL "Dev")
    if(NOT MSVC)
        add_definitions(-Werror -fno-omit-frame-pointer)
    endif()
    add_definitions(${OSMIUM_WARNING_OPTIONS})
endif()

# Force RelWithDebInfo build type if none was given
if(CMAKE_BUILD_TYPE)
    set(build_type ${CMAKE_BUILD_TYPE})
else()
    set(build_type "RelWithDebInfo")
endif()

set(CMAKE_BUILD_TYPE ${build_type}
    CACHE STRING
    "Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}."
    FORCE)

set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")

#-----------------------------------------------------------------------------

add_executable(osm-history-importer importer.cpp handler.hpp entitytracker.hpp nodestore.hpp nodestore/stl.hpp nodestore/sparse.hpp polygonidentifyer.hpp zordercalculator.hpp sorttest.hpp project.hpp)
message("DEBUG: PGLIB=${PostgreSQL_LIBRARY}")
message("DEBUG: PROJ4_LIB=${PROJ4_LIBRARY}")
message("DEBUG: GEOS_LIBRARY=${GEOS_LIBRARY}")
message("DEBUG: OSMIUM_LIBRARIES=${OSMIUM_LIBRARIES}")
target_link_libraries(osm-history-importer ${OSMIUM_LIBRARIES} ${PostgreSQL_LIBRARY} ${GEOS_LIBRARY})
# need copy and fix path scheme before install
# install(TARGETS osm-history-importer DESTINATION bin)



#-----------------------------------------------------------------------------
