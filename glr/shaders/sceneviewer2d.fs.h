"#version 330 core\n \
/*\n \
*\n \
*\n \
RESERVED\n \
*\n \
*\n \
*/\n \
uniform vec3 Kd;\n \
\n \
uniform sampler2D texture1;\n \
uniform int textureAssigned;\n \
/*\n \
*\n \
*\n \
END RESERVED\n \
*\n \
*\n \
*/\n \
\n \
in vec2 TexCoord;\n \
\n \
out vec4 FragColor;\n \
\n \
void main()\n \
{\n \
    // texture mapping\n \
    vec3 texColor = vec3(texture(texture1, TexCoord));\n \
    vec3 diffuseBase;\n \
    (textureAssigned == 0) ? (diffuseBase = Kd) : (diffuseBase = vec3(texColor));\n \
	FragColor = vec4(diffuseBase,1);\n \
}"
