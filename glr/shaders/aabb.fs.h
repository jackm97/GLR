"#version 330 core\n \
\n \
/*\n \
*\n \
*\n \
RESERVED\n \
*\n \
*\n \
*/\n \
uniform vec3 boxColor;\n \
/*\n \
*\n \
*\n \
END RESERVED\n \
*\n \
*\n \
*/\n \
\n \
in vec3 outColor;\n \
out vec4 FragColor;\n \
\n \
\n \
void main()\n \
{    \n \
    FragColor = vec4(outColor, 1.0);\n \
} \n \
"
