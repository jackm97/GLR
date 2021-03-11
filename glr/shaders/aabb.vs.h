"#version 330\n \
\n \
/*\n \
*\n \
*\n \
RESERVED\n \
*\n \
*\n \
*/\n \
layout (location=0) in vec3 inPos;\n \
layout (location=1) in vec3 inColor;\n \
  \n \
uniform mat4 m;\n \
uniform mat4 v;\n \
uniform mat4 p;\n \
/*\n \
*\n \
*\n \
END RESERVED\n \
*\n \
*\n \
*/\n \
\n \
out vec3 outColor;\n \
\n \
\n \
void main()\n \
{\n \
  gl_Position = vec4(inPos, 1.);\n \
  gl_Position = p * v * m * gl_Position;\n \
\n \
  outColor = inColor;\n \
}\n \
"
