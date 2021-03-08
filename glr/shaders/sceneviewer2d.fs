#version 330 core
/*
*
*
RESERVED
*
*
*/
uniform vec3 Kd;

uniform sampler2D texture1;
uniform int textureAssigned;
/*
*
*
END RESERVED
*
*
*/

in vec2 TexCoord;

out vec4 FragColor;

void main()
{
    // texture mapping
    vec3 texColor = vec3(texture(texture1, TexCoord));
    vec3 diffuseBase;
    (textureAssigned == 0) ? (diffuseBase = Kd) : (diffuseBase = vec3(texColor));
	FragColor = vec4(diffuseBase,1);
}
