cmake_minimum_required(VERSION 3.13.0)
project(jfs VERSION 0.1.0)

#####################################################################
# CUSTOM OPTIONS:
#
# GLM PATH:
#   If adding this as a CMake subdirectory
#   GLM_PATH can instead be set in the top
#   level CMakeLists.txt
if(NOT DEFINED GLM_PATH)    
    set(GLM_PATH ${PROJECT_SOURCE_DIR}/extern/glm)
endif()

# INSTALL PATH:
#   E.g. if /usr/local will install headers in /usr/local/include/glr
#   and library in /usr/local/lib/
#
#   Note that CMake doesn't have an uninstall feature
#   To uninstall, manual removal of the files is necessary
    
set(INSTALL_PATH ${PROJECT_SOURCE_DIR}/install)

# END CUSTOM OPTIONS
#####################################################################

include_directories(${PROJECT_SOURCE_DIR}/
                    ${GLM_PATH}/)

set(GLR_SOURCE_DIR ${PROJECT_SOURCE_DIR}/glr)

add_definitions(-DGLRENDER_STATIC)

add_library(glr STATIC ${GLR_SOURCE_DIR}/initialize.cpp
                       ${GLR_SOURCE_DIR}/shader.cpp
                       ${GLR_SOURCE_DIR}/texture.cpp
                       ${GLR_SOURCE_DIR}/renderBase.cpp
                       ${GLR_SOURCE_DIR}/sceneViewer.cpp
                       ${GLR_SOURCE_DIR}/sceneViewer2D.cpp )

set(GLR_HEADERS ${GLR_SOURCE_DIR}/static_build/glr_inline.h
                ${GLR_SOURCE_DIR}/glr_typedefs.h
                ${GLR_SOURCE_DIR}/initialize.h
                ${GLR_SOURCE_DIR}/shader.h
                ${GLR_SOURCE_DIR}/texture.h
                ${GLR_SOURCE_DIR}/renderBase.h
                ${GLR_SOURCE_DIR}/sceneViewer.h
                ${GLR_SOURCE_DIR}/sceneViewer2D.h )

install(TARGETS glr DESTINATION ${INSTALL_PATH}/lib/glr)
install(FILES ${GLR_HEADERS} DESTINATION ${INSTALL_PATH}/include/glr)
install(DIRECTORY ${GLM_PATH} DESTINATION ${INSTALL_PATH}/include/glr)


# GLAD PATH:
#   If adding this as a CMake subdirectory
#   GLAD_PATH can instead be set in the top
#   level CMakeLists.txt
set(GLAD_SRC_NAME glad.c) 
set(GLAD_PATH ${PROJECT_SOURCE_DIR}/extern/glad)
include_directories(${GLAD_PATH}/include)
add_library(glad_glr STATIC ${GLAD_PATH}/src/${GLAD_SRC_NAME})
target_link_libraries(glad_glr PRIVATE ${CMAKE_DL_LIBS})
target_link_libraries(glr PRIVATE glad_glr)
