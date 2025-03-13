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

out vec2 vs_tex;

uniform mat4 u_trm;

void
main()
{
    vs_tex = a_texCoords;
    gl_Position = u_trm * vec4(a_pos, 0.0, 1.0);
}
)";

static const char* ntsQuadTexFrag =
R"(#version 300 es
/* ntsQuadTexFrag */

precision mediump float;

out vec4 fs_fragColor;
in vec2 vs_tex;

uniform sampler2D u_tex0;

void
main()
{
    fs_fragColor = texture(u_tex0, vs_tex);
}
)";

static const char* ntsQuadTexMonoFrag =
R"(#version 300 es
/* ntsQuadTexMonoFrag */

precision mediump float;

out vec4 fs_fragColor;
in vec2 vs_tex;

uniform sampler2D u_tex0;
uniform vec4 u_color;

void
main()
{
    vec3 col = texture(u_tex0, vs_tex).rrr;
    fs_fragColor = vec4(col, 1.0f);
}
)";

static const char* ntsQuadTexMonoAlphaDiscardFrag =
R"(#version 300 es
/* ntsQuadTexMonoAlphaDiscardFrag */

precision mediump float;

out vec4 fs_fragColor;
in vec2 vs_tex;

uniform sampler2D u_tex0;
uniform vec4 u_color;

void
main()
{
    vec3 col = texture(u_tex0, vs_tex).rrr;
    if (col.r < 0.01) discard;

    fs_fragColor = u_color;
}
)";

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
)";

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
)";

static const char* ntsSimpleTextureVert =
R"(#version 300 es
/* ntsSimpleTextureVert */

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_tex;

out vec2 vs_tex;

uniform mat4 u_trm;

void
main()
{
    vs_tex = a_tex;
    gl_Position = u_trm * vec4(a_pos, 1.0);
}
)";

static const char* ntsSimpleTextureFrag =
R"(#version 300 es
/* ntsSimpleTextureFrag */

precision mediump float;

out vec4 fs_fragColor;

in vec2 vs_tex;

uniform sampler2D u_tex0;

void
main()
{
    vec4 color = texture(u_tex0, vs_tex);
    if (color.a <= 0.01) discard;

    fs_fragColor = color;
}
)";

static const char* ntsSkinTextureVert =
R"(#version 300 es
/* ntsSkinTestVert */

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_tex;
layout(location = 2) in vec4 a_joint;
layout(location = 3) in vec4 a_weight;

out vec4 vs_pos;
out vec2 vs_tex;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_a128TrmJoints[128];

void
main()
{
    mat4 trmSkin =
        a_weight.x * u_a128TrmJoints[int(a_joint.x)] +
        a_weight.y * u_a128TrmJoints[int(a_joint.y)] +
        a_weight.z * u_a128TrmJoints[int(a_joint.z)] +
        a_weight.w * u_a128TrmJoints[int(a_joint.w)];

    vec4 worldPos = trmSkin * vec4(a_pos, 1.0);

    gl_Position = u_projection * u_view * u_model * worldPos;

    vs_pos = a_weight;
    vs_tex = a_tex;
}
)";

static const char* ntsSkinVert =
R"(#version 300 es
/* ntsSkinVert */

layout(location = 0) in vec3 a_pos;
layout(location = 2) in vec4 a_joint;
layout(location = 3) in vec4 a_weight;

out vec4 vs_pos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_a128TrmJoints[128];

void
main()
{
    mat4 trmSkin =
        a_weight.x * u_a128TrmJoints[int(a_joint.x)] +
        a_weight.y * u_a128TrmJoints[int(a_joint.y)] +
        a_weight.z * u_a128TrmJoints[int(a_joint.z)] +
        a_weight.w * u_a128TrmJoints[int(a_joint.w)];

    vec4 worldPos = trmSkin * vec4(a_pos, 1.0);

    gl_Position = u_projection * u_view * u_model * worldPos;

    vs_pos = a_weight;
}
)";

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
)";

static const char* ntsSkyboxVert =
R"(#version 300 es
/* ntsSkyboxVert */

precision mediump float;

layout (location = 0) in vec3 a_pos;

uniform mat4 u_viewNoTranslate;
uniform mat4 u_projection;

out vec3 vs_tex;

void
main()
{
    vs_tex = a_pos;
    gl_Position = u_projection * u_viewNoTranslate * vec4(a_pos, 1.0);
}
)";

static const char* ntsSkyboxFrag =
R"(#version 300 es
/* ntsSkyboxFrag */

precision mediump float;

in vec3 vs_tex;

uniform samplerCube u_tex0;

out vec4 fragColor;

void
main()
{
    fragColor = texture(u_tex0, vs_tex);
}
)";

static const char* nts2DVert =
R"(#version 300 es
/* nts2DVert */

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex;

uniform mat4 u_projection;

out vec2 vs_tex;

void main()
{
    vs_tex = a_tex;
    gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);
}
)";

static const char* nts2DColorFrag =
R"(#version 300 es
/* nts2DColorFrag */

precision mediump float;

out vec4 fs_fragColor;

in vec2 vs_tex;

uniform vec4 u_color;

void
main()
{
    if (u_color.a <= 0.01) discard;

    fs_fragColor = u_color;
}
)";

} /* namespace gl::glsl */
