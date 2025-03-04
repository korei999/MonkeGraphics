#pragma once

namespace shaders::glsl
{

constexpr int POS_LOCATION = 0;
constexpr int TEX_LOCATION = 1;
constexpr int JOINT_LOCATION = 2;
constexpr int WEIGHT_LOCATION = 3;

static const char* ntsQuadTexVert =
R"(#version 300 es
/* ntsQuadTexVert */

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
R"(#version 300 es
/* ntsQuadTexFrag */

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

static const char* ntsSimpleColorVert =
R"(#version 300 es
/* ntsSimpleColorVert */

layout(location = 0) in vec3 a_pos;

uniform mat4 u_trm;

void
main()
{
    gl_Position = u_trm * vec4(a_pos, 1.0);
}
)"
;

static const char* ntsSimpleColorFrag =
R"(#version 300 es
/* ntsSimpleColorFrag */

precision mediump float;

out vec4 fs_fragColor;

uniform vec4 u_color;

void
main()
{
    fs_fragColor = u_color;
}
)"
;

static const char* ntsSimpleTextureVert =
R"(#version 300 es
/* ntsSimpleTextureVert */

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_tex;

out vec2 vs_texCoords;

uniform mat4 u_trm;

void
main()
{
    vs_texCoords = a_tex;
    gl_Position = u_trm * vec4(a_pos, 1.0);
}
)"
;

static const char* ntsSimpleTextureFrag =
R"(#version 300 es
/* ntsSimpleTextureFrag */

precision mediump float;

out vec4 fs_fragColor;

in vec2 vs_texCoords;

uniform sampler2D u_tex0;

void
main()
{
    fs_fragColor = texture(u_tex0, vs_texCoords);
}
)"
;

static const char* ntsSkinTestVert =
R"(#version 300 es
/* ntsSkinTestVert */

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_tex;
layout(location = 2) in vec4 a_joint;
layout(location = 3) in vec4 a_weight;

out vec4 vs_pos;
out vec2 vs_texCoords;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_a2TrmJoints[128];

void
main()
{
    mat4 trmSkin =
        a_weight.x * u_a2TrmJoints[int(a_joint.x)] +
        a_weight.y * u_a2TrmJoints[int(a_joint.y)] +
        a_weight.z * u_a2TrmJoints[int(a_joint.z)] +
        a_weight.w * u_a2TrmJoints[int(a_joint.w)];

    vec4 worldPos = trmSkin * vec4(a_pos, 1.0);

    gl_Position = u_projection * u_view * u_model * worldPos;

    vs_pos = a_weight;
    vs_texCoords = a_tex;
}
)"
;

static const char* ntsInterpolatedColorFrag =
R"(#version 300 es
/* ntsInterpolatedColorFrag */

precision mediump float;

in vec4 vs_pos;

out vec4 fs_fragColor;

void
main()
{
    fs_fragColor = vs_pos;
}
)"
;

} /* namespace gl::glsl */

