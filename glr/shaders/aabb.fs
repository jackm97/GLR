#version 330 core

/*
*
*
RESERVED
*
*
*/
uniform vec3 boxColor;
/*
*
*
END RESERVED
*
*
*/

in vec3 outColor;
out vec4 FragColor;


void main()
{    
    FragColor = vec4(outColor, 1.0);
} 