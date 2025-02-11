/* src/shaders/quad/tex.vert */

#version 300 es

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_texCoords;

out vec2 vs_texCoords;

void
main()
{
    vs_texCoords = a_texCoords;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
