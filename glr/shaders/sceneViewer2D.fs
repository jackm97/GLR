#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;

vec4 colormap(float x);

void main()
{
	FragColor = texture(texture1, TexCoord);
}
