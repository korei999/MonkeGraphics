#pragma once

namespace shaders::glsl
{

static const char* ntsQuadTexVert =
R"(/* ntsQuadTexVert */

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
)"
;

static const char* ntsQuadTexFrag =
R"(/* ntsQuadTexFrag */

#version 300 es

precision mediump float;

out vec4 fs_fragColor;
in vec2 vs_texCoords;

uniform sampler2D u_tex0;

void
main()
{
    fs_fragColor = texture(u_tex0, vs_texCoords);
    /*fs_fragColor = vec4(vs_texCoords, 0.0, 1.0);*/
}
)"
;

} /* namespace gl::glsl */

