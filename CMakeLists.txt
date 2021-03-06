CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT( libquadtree )

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake) 

#=============================================================================
# Set the output directories for the binary and library files
#=============================================================================

#=============================================================================
# unknown block purpose
#=============================================================================
# set(CMakeScripts_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# set(CMAKE_MODULE_PATH ${CMakeScripts_MODULE_DIR})

#=============================================================================
# Specify Compiler Flags
#=============================================================================

#---------------------------------------------
# Linux and Apple Compiler Flags
#---------------------------------------------
# Flags for the GNU C++ Compiler
SET( WALL_ON OFF CACHE BOOL  "tell me about all compiler warnings (-Wall) ")
IF(WALL_ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" -C++11)
ENDIF( WALL_ON)


# ============= libpng =================
# (Enables output of graph to images)
# SET(LIBPNG_LINKAGE "/usr/local/Cellar/libpng/1.6.37/lib/libpng.dylib")
# ADD_DEFINITIONS(-DENABLE_LIBPNG)

# # want to migrate from libpng -> gdal (see below)
# SET(LIBRARY_LINKAGE ${LIBPNG_LINKAGE})
# SET(LIBRARY_LINKAGE ${LIBRARY_LINKAGE} png)
 #MESSAGE( STATUS "    with path: ${GDAL_PATH}")

# ============= GDal =================
# Used for most file I/O
# -- Assumed to be already installed:

SET(GDAL_PATH "/usr/local/Cellar/gdal/2.4.2/lib/")
# SET(GDAL_LINKAGE "/usr/local/Cellar/gdal/2.4.2/lib/libgdal.dylib")
SET(GDAL_LINKAGE gdal)
SET(LIBRARY_LINKAGE ${LIBRARY_LINKAGE} ${GDAL_LINKAGE})
ADD_DEFINITIONS(-DENABLE_GDAL)

MESSAGE( STATUS "Configured GDal... ")
MESSAGE( STATUS "    with path: ${GDAL_PATH}")

#=============================================================================
# Add Subdirectories
#=============================================================================
include_directories(include)
SET(LIB_HEADERS include/terrain.hpp include/terrain.inl
                include/geometry/interpolate.hpp
                include/geometry/layout.hpp
                include/geometry/polygon.hpp
                include/grid/grid.hpp
                include/io/json.hpp
                include/io/readers.hpp include/io/readers.inl
                include/io/writers.hpp include/io/writers.inl
                include/quadtree/node.hpp
                include/quadtree/tree.hpp)

SET(LIB_SOURCES src/terrain.cpp
                src/geometry/interpolate.cpp
                src/geometry/layout.cpp
                src/geometry/polygon.cpp
                src/grid/grid.cpp
                src/quadtree/node.cpp
                src/quadtree/tree.cpp
                )

# ============= Quad Tree Library =================
SET(QUAD_TREE_LIB_NAME quadtree)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -g -pedantic -Iinclude/")
IF(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g")
ENDIF (CMAKE_BUILD_TYPE EQUAL "DEBUG")
IF(CMAKE_BUILD_TYPE EQUAL "RELEASE")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2")
ENDIF (CMAKE_BUILD_TYPE EQUAL "RELEASE")


INCLUDE_DIRECTORIES(vendor)

#Generate the static library from the sources
add_library(${QUAD_TREE_LIB_NAME} STATIC ${LIB_SOURCES})

TARGET_LINK_LIBRARIES(${QUAD_TREE_LIB_NAME} PRIVATE ${LIBRARY_LINKAGE})

# ============= Enable Testing  =================

#ExternalProject_Add(GTest
#    TMP_DIR ${PROJECT_SOURCE_DIR}/vendor/googletest 
#    GIT_REPOSITORY https://github.com/google/googletest.git) 
#
# INCLUDE_DIRECTORIES(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR}) 

enable_testing()

# ============= Build Project Tests  =================
# https://cmake.org/cmake/help/v3.0/module/FindGTest.html 
SET(TEST_EXE testtree) 
SET(TEST_SOURCES
                    test/geometry/interpolate.cpp
                    test/geometry/layout.cpp
                    test/geometry/polygon.cpp
                    test/grid/grid.cpp
                    test/quadtree/node.cpp
                    test/quadtree/tree.cpp                    )
                    
SET(TEST_LINKAGE ${QUAD_TREE_LIB_NAME} ${LIBRARY_LINKAGE} gtest gtest_main ) 

MESSAGE( STATUS "Generating tests for ${LIB_NAME}") 
MESSAGE( STATUS "    with name: ${TEST_EXE}") 
MESSAGE( STATUS "    with sources: ${TEST_SOURCES}") 
MESSAGE( STATUS "    with linkage: ${TEST_LINKAGE}") 

ADD_EXECUTABLE( ${TEST_EXE} ${TEST_SOURCES}) 
ADD_TEST(AllTestsInFoo ${TEST_EXE}) 

TARGET_COMPILE_OPTIONS(${TEST_EXE} PRIVATE -std=c++17 -Wall -g -pedantic -Iinclude/*) 

TARGET_LINK_LIBRARIES(${TEST_EXE} ${TEST_LINKAGE})


# ============= Build Profiling Program  =================
SET(PROFILE_EXE proftree)
SET(PROFILE_SOURCES src/profile/main.cpp)

# We don't YET need this:
# SET(PROFILE_LINKAGE ${QUAD_TREE_LIB_NAME} ${GDAL_LINKAGE})


MESSAGE( STATUS "Generating profiling program: ${PROFILE_EXE}")
MESSAGE( STATUS "    with sources: ${PROFILE_SOURCES}")
MESSAGE( STATUS "    with linkage: ${PROFILE_LINKAGE}") 

ADD_EXECUTABLE( ${PROFILE_EXE} ${PROFILE_SOURCES})
TARGET_INCLUDE_DIRECTORIES(${PROFILE_EXE} PRIVATE vendor)
TARGET_LINK_LIBRARIES(${PROFILE_EXE} PRIVATE ${QUAD_TREE_LIB_NAME} ${PROFILE_LINKAGE}) 
TARGET_COMPILE_OPTIONS(${PROFILE_EXE} PRIVATE -std=c++17 -Wall -g -pedantic -Iinclude/*)
