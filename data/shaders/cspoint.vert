#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#pragma import_defines (VSGCS_BILLBOARD_NORMAL)

#include "descriptor_defs.glsl"

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
} pc;


layout(location = 0) in vec3 vsg_Vertex;
layout(location = 1) in vec3 vsg_Normal;
layout(location = 2) in vec4 vsg_Color;
layout(location = 4) in vec2 vsg_TexCoord[4];

layout(location = 0) out vec3 eyePos;
layout(location = 1) out vec3 normalDir;

layout(location = 2) out vec4 vertexColor;
layout(location = 3) out vec3 viewDir;
layout(location = 4) out vec2 texCoord[4];


out gl_PerVertex{ vec4 gl_Position; float gl_PointSize; };

// Texture coordinates are assumed to have the OpenGL / glTF origin i.e., lower left.
vec4 cstexture(sampler2D texmap, vec2 coords)
{
    return texture(texmap, vec2(coords.s, 1.0 - coords.t));
}

// VSG_BILLBOARD_NORMAL orients the normal to face the viewer.


void main()
{
    vec4 vertex = vec4(vsg_Vertex, 1.0);
    vec4 normal = vec4(vsg_Normal, 0.0);

    gl_Position = (pc.projection * pc.modelView) * vertex;
    gl_PointSize = 1.0;
    eyePos = (pc.modelView * vertex).xyz;

    viewDir = - (pc.modelView * vertex).xyz;
#ifdef VSGCS_BILLBOARD_NORMAL
    normalDir = eyePos;         // -viewDir
#else
    normalDir = (pc.modelView * normal).xyz;
#endif
    vertexColor = vsg_Color;
    for (int i = 0; i < 4; i++)
    {
        texCoord[i] = vsg_TexCoord[i];
    }
}
