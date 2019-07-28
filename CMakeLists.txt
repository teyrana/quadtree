CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT( libquadtree )

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

#=============================================================================
# Add Subdirectories
#=============================================================================
include_directories(include)
SET(LIB_HEADERS include/bounds.hpp
                            include/point.hpp
                            include/quad_node.hpp
                            include/quad_tree.hpp)
  
SET(LIB_SOURCES src/bounds.cpp
                            src/point.cpp
                            src/quad_node.cpp
                            src/quad_tree.cpp)


# ============= Quad Tree Library =================
SET(LIB_NAME quadtree)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -g -pedantic -Iinclude/ -std=c++17 -Wall -g -pedantic -Iinclude/")
IF(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g")
ENDIF (CMAKE_BUILD_TYPE EQUAL "DEBUG")
IF(CMAKE_BUILD_TYPE EQUAL "RELEASE")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2")
ENDIF (CMAKE_BUILD_TYPE EQUAL "RELEASE")


#Generate the static library from the sources
add_library(${LIB_NAME} STATIC ${LIB_SOURCES})

# ============= Test Executables =================
SET(TEST_APP testtree)
SET(TEST_SOURCES test.cpp) 


ADD_EXECUTABLE( ${TEST_APP} ${TEST_SOURCES})
TARGET_LINK_LIBRARIES( ${TEST_APP} ${LIB_NAME})

