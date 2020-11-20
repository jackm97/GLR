#version 330 core
/*
*
*
RESERVED
*
*
*/
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

uniform float boundX, boundY;

uniform mat4 m;
/*
*
*
END RESERVED
*
*
*/

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(inPos, 1.0);
	gl_Position.x /= boundX;
	gl_Position.y /= boundY;
	gl_Position = m * gl_Position;
	TexCoord = vec2(inUV.x, inUV.y);
}
