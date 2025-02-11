/* src/shaders/quad/tex.frag */

#version 300 es

precision mediump float;

out vec4 fs_fragColor;
in vec2 vs_texCoords;

uniform sampler2D u_tex0;

void
main()
{
    fs_fragColor = texture(u_tex0, vs_texCoords);
    // fs_fragColor = vec4(vs_texCoords, 0.0, 1.0);
}
