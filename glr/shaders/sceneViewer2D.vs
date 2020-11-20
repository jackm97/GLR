"#version 330 core\n \
layout (location = 0) in vec3 aPos;\n \
layout (location = 1) in vec2 aTexCoord;\n \
\n \
out vec2 TexCoord;\n \
\n \
void main()\n \
{\n \
	gl_Position = vec4(aPos, 1.0);\n \
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n \
}"
