"#version 330 core\n \
/*\n \
*\n \
*\n \
RESERVED\n \
*\n \
*\n \
*/\n \
layout (location = 0) in vec3 inPos;\n \
layout (location = 1) in vec2 inUV;\n \
\n \
uniform float boundX, boundY;\n \
uniform int isBackground;\n \
\n \
uniform mat4 m;\n \
/*\n \
*\n \
*\n \
END RESERVED\n \
*\n \
*\n \
*/\n \
\n \
out vec2 TexCoord;\n \
\n \
void main()\n \
{\n \
	gl_Position = vec4(inPos, 1.0);\n \
\n \
	bool isBackgroundBool = (isBackground == 1);\n \
	(!isBackgroundBool) ? (gl_Position.x -= boundX/2) : (gl_Position.x = gl_Position.x);\n \
	(!isBackgroundBool) ? (gl_Position.y -= boundY/2) : (gl_Position.y = gl_Position.y);\n \
	(!isBackgroundBool) ? (gl_Position.x /= (boundX/2)) : (gl_Position.x = gl_Position.x);\n \
	(!isBackgroundBool) ? (gl_Position.y /= (boundY/2)) : (gl_Position.y = gl_Position.y);\n \
\n \
	gl_Position = m * gl_Position;\n \
	TexCoord = vec2(inUV.x, inUV.y);\n \
}"
