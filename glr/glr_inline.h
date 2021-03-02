#ifdef GLRENDER_INLINE
#undef GLRENDER_INLINE
#endif

#ifndef GLRENDER_STATIC
#  define GLRENDER_INLINE inline
#else
#  define GLRENDER_INLINE
#  include <glr/initialize.h>
#endif

#include <glr/glr_typedefs.h>