#version 330

in vec2 tex_coords;
out vec4 OutColor;

uniform sampler2D u_frameTex;

void main()
{
    vec3 prevColor = texture(u_frameTex, tex_coords).rgb;
    OutColor = vec4(prevColor, 1.0);
}