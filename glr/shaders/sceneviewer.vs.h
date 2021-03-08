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
layout (location=1) in vec3 inNorm;\n \
layout (location=2) in vec3 inVertColor;\n \
layout (location=3) in vec2 inUV;\n \
  \n \
uniform mat4 m;\n \
uniform mat4 v;\n \
uniform mat4 p;\n \
\n \
uniform vec3 shapeCenter;\n \
/*\n \
*\n \
*\n \
END RESERVED\n \
*\n \
*\n \
*/\n \
\n \
out vec3 FragPos;\n \
out vec3 Norm;\n \
out vec3 VertColor;\n \
out vec2 TexCoord;\n \
\n \
\n \
void main()\n \
{\n \
  gl_Position = vec4(inPos, 1.);\n \
  gl_Position = p * v * m * gl_Position;\n \
\n \
  FragPos = vec3(m * vec4(inPos, 1.0));\n \
  Norm = mat3(m) * inNorm;\n \
  VertColor = inVertColor;\n \
  TexCoord = inUV;\n \
}\n \
"
