#version 330

/*
*
*
RESERVED
*
*
*/
layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inColor;
  
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
/*
*
*
END RESERVED
*
*
*/

out vec3 outColor;


void main()
{
  gl_Position = vec4(inPos, 1.);
  gl_Position = p * v * m * gl_Position;

  outColor = inColor;
}