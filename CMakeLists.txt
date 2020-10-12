cmake_minimum_required(VERSION 2.8.6)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_PREFIX ${INSTALL})
cmake_policy(SET CMP0017 NEW)
cmake_policy(SET CMP0020 NEW)

project("cbmrnp" C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

#if(CMAKE_COMPILER_IS_GNUCXX)
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -Wextra")
#endif()

# Required libraries
find_package(Qt5OpenGL 5.4.0 REQUIRED)

#process resource files
############################
# List your qrc files here #
# (if you need any)        #
############################
qt5_add_resources (ResourceSources shader.qrc)

# Optional libraries
find_package(GTA QUIET)

# The utility library
add_subdirectory(glbase)

# The program
################################
# List your project files here #
################################
add_executable(cbmrnp
    main.cpp
    ${ResourceSources}
    gui/glwidget.cpp
    gui/glwidget.hpp
    gui/cli.cpp
    gui/cli.h

    objects/drawable.cpp
#    objects/skybox.cpp
    objects/spacetime.cpp

    image/image.cpp
    image/image.h

    shader/spacetime.fs.glsl
    shader/spacetime.vs.glsl

)


include_directories(${CMAKE_SOURCE_DIR}/glbase ${OPENGL_INCLUDE_DIR})

####################################
# Build with correct opengl lib    #
# (this could fail on your system) #
####################################
if(WIN32 OR CYGWIN)
        target_link_libraries(cbmrnp opengl32 libglbase Qt5::OpenGL ${OPENGL_gl_LIBRARY})
else()
        target_link_libraries(cbmrnp GL libglbase Qt5::OpenGL ${OPENGL_gl_LIBRARY})
endif()

if(GTA_FOUND)
        add_definitions(-DHAVE_GTA)
        include_directories(${GTA_INCLUDE_DIR})
        target_link_libraries(cbmrnp ${GTA_LIBRARIES})
endif()

install(TARGETS cbmrnp RUNTIME DESTINATION bin)
