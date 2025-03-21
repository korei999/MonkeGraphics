#pragma once

namespace shaders::glsl
{

constexpr int POS_LOCATION = 0;
constexpr int TEX_LOCATION = 1;
constexpr int JOINT_LOCATION = 2;
constexpr int WEIGHT_LOCATION = 3;
constexpr int NORMAL_LOCATION = 4;

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

static const char* ntsQuadTexColorFrag =
R"(#version 300 es
/* ntsQuadTexFrag */

precision mediump float;

out vec4 fs_fragColor;
in vec2 vs_tex;

uniform sampler2D u_tex0;
uniform vec4 u_color;

void
main()
{
    fs_fragColor = texture(u_tex0, vs_tex) * u_color;
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
    if (col.r < 0.01) discard;

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

static const char* ntsQuadTexMonoBoxBlurFrag =
R"(#version 300 es
/* ntsQuadTexMonoBoxBlurFrag */

precision mediump float;

out vec4 fs_fragColor;
in vec2 vs_tex;

uniform sampler2D u_tex0;
uniform vec4 u_color;
uniform vec2 u_texelSize;

vec3
boxBlur()
{
    vec3 sum = vec3(0.0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 offset = vec2(x, y) * u_texelSize;
            sum += texture(u_tex0, vs_tex + offset).rrr;
        }
    }

    return sum;
}

void
main()
{
    vec3 col = texture(u_tex0, vs_tex).rrr;

    fs_fragColor = vec4(boxBlur(), col.r) * u_color;
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
layout(location = 2) in ivec4 a_joint;
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
        a_weight.x * u_a128TrmJoints[a_joint.x] +
        a_weight.y * u_a128TrmJoints[a_joint.y] +
        a_weight.z * u_a128TrmJoints[a_joint.z] +
        a_weight.w * u_a128TrmJoints[a_joint.w];

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
layout(location = 2) in ivec4 a_joint;
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
        a_weight.x * u_a128TrmJoints[a_joint.x] +
        a_weight.y * u_a128TrmJoints[a_joint.y] +
        a_weight.z * u_a128TrmJoints[a_joint.z] +
        a_weight.w * u_a128TrmJoints[a_joint.w];

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

static const char* ntsGouraudVert =
R"(#version 320 es
/* ntsGouraudVert */

precision mediump float;

layout(location = 0) in vec3 a_pos;
layout(location = 2) in ivec4 a_joint;
layout(location = 3) in vec4 a_weight;
layout(location = 4) in vec3 a_norm;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec4 u_color;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform vec3 u_ambientColor;
uniform mat4 u_a128TrmJoints[128];

out vec4 vs_color;

void main()
{
    mat4 trmSkin =
        a_weight.x * u_a128TrmJoints[a_joint.x] +
        a_weight.y * u_a128TrmJoints[a_joint.y] +
        a_weight.z * u_a128TrmJoints[a_joint.z] +
        a_weight.w * u_a128TrmJoints[a_joint.w];

    mat4 finalTrm = u_model * trmSkin;

    vec4 worldPos = finalTrm * vec4(a_pos, 1.0);

    gl_Position = u_projection * u_view * worldPos;

    mat3 normalTrm = transpose(inverse(mat3(finalTrm)));

    vec3 norm = normalize(normalTrm * a_norm);
    vec3 lightDir = normalize(u_lightPos - worldPos.xyz);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_lightColor;

    vs_color = vec4((u_ambientColor + diffuse), 1.0) * u_color;
}
)";

static const char* ntsGouraudFrag =
R"(#version 320 es
/* ntsGouraudFrag */

precision mediump float;

in vec4 vs_color;

out vec4 fs_color;

void main()
{
    fs_color = vs_color;
}
)";

} /* namespace gl::glsl */
