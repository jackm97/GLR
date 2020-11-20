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
	gl_Position.x /= boundX;\n \
	gl_Position.y /= boundY;\n \
	gl_Position = m * gl_Position;\n \
	TexCoord = vec2(inUV.x, inUV.y);\n \
}"
