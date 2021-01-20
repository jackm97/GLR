This is my own custom fork of the stb image library found [here](https://github.com/nothings/stb).

My goal is to namespace some of the files for my own personaly libraries so that there are no conflicts when someone uses the main fork of the stb image library in their project along with my library.

So far I have only namespaced stb_image.h.

This is no longer a C library since it is namespaced. Only C++ now.

To define custom namespace use #define STB_CUSTOM_NAMESPACE my_namespace.

Instead of using #define STB_IMAGE_IMPLEMENTATION, use #define CUSTOM_STB_IMAGE_IMPLEMENTATION
