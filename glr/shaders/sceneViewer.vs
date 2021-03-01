#version 330

/*
*
*
RESERVED
*
*
*/
layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inNorm;
layout (location=2) in vec2 inUV;
  
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

uniform vec3 shapeCenter;
/*
*
*
END RESERVED
*
*
*/

out vec3 FragPos;
out vec3 Norm;
out vec2 TexCoord;


void main()
{
  gl_Position = vec4(inPos, 1.);
  gl_Position = p * v * m * gl_Position;

  FragPos = vec3(m * vec4(inPos, 1.0));
  Norm = mat3(m) * inNorm;
  TexCoord = inUV;
}