cmake_minimum_required(VERSION 3.13.0)
project(jfs VERSION 0.1.0)

#####################################################################
# CUSTOM OPTIONS:
#
# GLAD PATH:
#   If adding this as a CMake subdirectory
#   GLAD_PATH can instead be set in the top
#   level CMakeLists.txt
# set(GLAD_SRC_NAME glad.c) 
if(NOT DEFINED GLAD_PATH)   
    set(GLAD_PATH ${PROJECT_SOURCE_DIR}/extern/glad)
endif()

# GLM PATH:
#   If adding this as a CMake subdirectory
#   GLM_PATH can instead be set in the top
#   level CMakeLists.txt
if(NOT DEFINED GLM_PATH)    
    set(GLM_PATH ${PROJECT_SOURCE_DIR}/extern/glm)
endif()

# EIGEN PATH:
#   If adding this as a CMake subdirectory
#   EIGEN_PATH can instead be set in the top
#   level CMakeLists.txt
if(NOT DEFINED EIGEN_PATH)    
    set(EIGEN_PATH ${PROJECT_SOURCE_DIR}/extern/eigen)
endif()

# END CUSTOM OPTIONS
#####################################################################

include_directories(${PROJECT_SOURCE_DIR}/
                    ${GLAD_PATH}/include
                    ${GLM_PATH}/
                    ${EIGEN_PATH}/)

set(GLR_SOURCE_DIR ${PROJECT_SOURCE_DIR}/glr)

add_definitions(-DGLRENDER_STATIC)

add_library(glr STATIC ${GLR_SOURCE_DIR}/initialize.cpp
                       ${GLR_SOURCE_DIR}/shader.cpp
                       ${GLR_SOURCE_DIR}/texture.cpp
                       ${GLR_SOURCE_DIR}/obj.cpp
                       ${GLR_SOURCE_DIR}/aabb_tree.cpp
                       ${GLR_SOURCE_DIR}/obb_tree.cpp
                       ${GLR_SOURCE_DIR}/renderbase.cpp
                       ${GLR_SOURCE_DIR}/sceneviewer.cpp
                       ${GLR_SOURCE_DIR}/sceneviewer2d.cpp)

set(GLR_HEADERS ${GLR_SOURCE_DIR}/static_build/glr_inline.h
                ${GLR_SOURCE_DIR}/initialize.h
                ${GLR_SOURCE_DIR}/shader.h
                ${GLR_SOURCE_DIR}/texture.h
                ${GLR_SOURCE_DIR}/obj.h
                ${GLR_SOURCE_DIR}/aabb_tree.h
                ${GLR_SOURCE_DIR}/obb_tree.h
                ${GLR_SOURCE_DIR}/renderbase.h
                ${GLR_SOURCE_DIR}/sceneviewer.h
                ${GLR_SOURCE_DIR}/sceneviewer2d.h )
