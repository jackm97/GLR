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

out vec4 FragColor;


void main()
{    
    FragColor = vec4(boxColor, 1.0);
} 