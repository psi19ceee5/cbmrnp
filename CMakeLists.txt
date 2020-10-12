# Copyright (C) 2013, 2014
# Computer Graphics Group, University of Siegen
# Written by Martin Lambers <martin.lambers@uni-siegen.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set(PROJECT_NAME cbmrnp)

project(${PROJECT_NAME})

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
cmake_policy(SET CMP0017 NEW)
cmake_policy(SET CMP0020 NEW)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -Wextra")
endif()

# Required libraries
find_package(Qt5OpenGL REQUIRED)
find_package(OpenGL REQUIRED)

# Optional libraries
find_package(GTA QUIET)

# The utility library
add_subdirectory(glbase)

# The program
include(StringifyShaders)
stringify_shaders(vs.glsl fs.glsl)
add_executable(${PROJECT_NAME} main.cpp GLWidget.hpp GLWidget.cpp vs.glsl fs.glsl vs.glsl.h fs.glsl.h)
include_directories(${CMAKE_SOURCE_DIR}/glbase ${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} libglbase Qt5::OpenGL ${OPENGL_gl_LIBRARY})
if(GTA_FOUND)
	add_definitions(-DHAVE_GTA)
	include_directories(${GTA_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${GTA_LIBRARIES})
endif()
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
