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
uniform int isBackground;

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
	
	bool isBackgroundBool = (isBackground == 1);
	(!isBackgroundBool) ? (gl_Position.x -= boundX/2) : (gl_Position.x = gl_Position.x);
	(!isBackgroundBool) ? (gl_Position.y -= boundY/2) : (gl_Position.y = gl_Position.y);
	(!isBackgroundBool) ? (gl_Position.x /= (boundX/2)) : (gl_Position.x = gl_Position.x);
	(!isBackgroundBool) ? (gl_Position.y /= (boundY/2)) : (gl_Position.y = gl_Position.y);
	
	gl_Position = m * gl_Position;
	TexCoord = vec2(inUV.x, inUV.y);
}
