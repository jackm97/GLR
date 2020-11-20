"#version 330 core\n \
out vec4 FragColor;\n \
\n \
in vec2 TexCoord;\n \
 \
// texture samplers\n \
uniform sampler2D texture1;\n \
\n \
void main()\n \
{\n \
	FragColor = texture(texture1, TexCoord);\n \
}"
