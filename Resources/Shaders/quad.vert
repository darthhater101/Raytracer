#version 330 core

const vec2 quad_vertices[4] = vec2[4]( vec2( -1.0, -1.0), vec2( 1.0, -1.0), vec2( -1.0, 1.0), vec2( 1.0, 1.0));
const vec2 tex_vertices[4] = vec2[4]( vec2( 0.0, 0.0), vec2( 1.0, 0.0), vec2( 0.0, 1.0), vec2( 1.0, 1.0));

out vec4 pix_coords;
out vec2 tex_coords;

void main()
{
	gl_Position = vec4(quad_vertices[gl_VertexID], 0.0, 1.0);
	pix_coords = gl_Position;
	tex_coords = vec2(tex_vertices[gl_VertexID]);
}