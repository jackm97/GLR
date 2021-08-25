# GLR
GLRenders: c++ library of various OpenGL rendering stuff I use in my projects.

It used to have header-only functionality but it's too difficult to maintain. There are two main ways of using the library. Building it and linking it manually or including at as a subdirectory in a cmake file.

## Dependencies
All the dependencies are included as git submodules in the `extern` folder of this repository. If you're project already includes these dependencies you will need to change their locations in the [CMakeLists.txt](./CMakeLists.txt).

- [glm](https://github.com/g-truc/glm)
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
- [glad](https://glad.dav1d.de/)
- [CMake](https://cmake.org/) - not a submodule, just needed to build the library


## Building the Library

### Windows
```
git clone --recursive https://github.com/jackm97/GLR
cd GLR
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Unix
```
git clone --recursive https://github.com/jackm97/GLR
cd GLR
mkdir build
cd build
cmake ..
make
```

Once built the `glr` library can be linked with any project. Make sure the repository is in the include path along with all dependencies. Additionally, the `glad.c` or `glad.cpp` source code needs to be built somewhere along the line, whether that's in the application that is linking with the `glr` library or as a separate library that is linked.

## CMake Subdirectory
To include the glr library in a CMake project just include the following in the top-level `CMakeLists.txt`:

```
#########################LIBGLR########################
# SET SOME OPTIONS FOR LIBGLR
#   GLR PATH
set(PATH_TO_GLR ${PROJECT_SOURCE_DIR}/extern/GLR/)
#   EXTERNAL LIB PATHS
set(GLM_PATH ${PATH_TO_GLR}/extern/glm)
set(EIGEN_PATH ${PROJECT_SOURCE_DIR}/extern/GLR/extern/eigen)
set(GLAD_PATH ${PROJECT_SOURCE_DIR}/extern/glad)

add_definitions(-DGLRENDER_STATIC)
add_subdirectory(${PATH_TO_GLR})

include_directories(${PATH_TO_GLR}
                    ${GLM_PATH}
                    ${EIGEN_PATH})
#########################LIBGLR########################
```

Just like building the library, the `glad.c` or `glad.cpp` source code needs to be built somewhere. One way is as a library:

``` 
#########################GLAD########################
set(GLAD_SRC_NAME glad.c)
include_directories(${GLAD_PATH}/include)
add_library(glad STATIC ${GLAD_PATH}/src/${GLAD_SRC_NAME})
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})
#########################GLAD########################
```

Link this library in the `target_link_libraries` call with the `glr` library.
