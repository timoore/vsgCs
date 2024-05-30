#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable 

#pragma import_defines (VSGCS_INSTANCES, VSG_DISPLACEMENT_MAP, VSGCS_FLAT_SHADING, VSGCS_BILLBOARD_NORMAL)

#include "descriptor_defs.glsl"

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
} pc;

#ifdef VSG_DISPLACEMENT_MAP
layout(set = PRIMITIVE_DESCRIPTOR_SET, binding = 6) uniform sampler2D displacementMap;
#endif

layout(location = 0) in vec3 vsg_Vertex;
layout(location = 1) in vec3 vsg_Normal;
layout(location = 2) in vec4 vsg_Color;
layout(location = 3) in vec2 vsg_TexCoord[4];
#ifdef VSGCS_INSTANCES
layout(location = 7) in mat3x4 vsgcs_InstanceMat;
#endif

layout(location = 0) out vec3 eyePos;
#ifdef VSGCS_FLAT_SHADING
layout(location = 1) flat out vec3 normalDir;
#else
layout(location = 1) out vec3 normalDir;
#endif
layout(location = 2) out vec4 vertexColor;
layout(location = 3) out vec3 viewDir;
layout(location = 4) out vec2 texCoord[4];


out gl_PerVertex{ vec4 gl_Position; };

// Texture coordinates are assumed to have the OpenGL / glTF origin i.e., lower left.
vec4 cstexture(sampler2D texmap, vec2 coords)
{
    return texture(texmap, vec2(coords.s, 1.0 - coords.t));
}

// VSG_BILLBOARD_NORMAL orients the normal to face the viewer.

void displaceGeometry(vec3 vertex, vec3 normal, out vec3 newVertex, out vec3 newNormal)
{
    newVertex = vertex;
    newNormal = normal;
#ifdef VSG_DISPLACEMENT_MAP
    // TODO need to pass as as uniform or per instance attributes
    vec3 scale = vec3(1.0, 1.0, 1.0);

    newVertex += normal * (cstexture(displacementMap, vsg_TexCoord[0]).s * scale.z);

    float s_delta = 0.01;
    float width = 0.0;

    float s_left = max(vsg_TexCoord[0].s - s_delta, 0.0);
    float s_right = min(vsg_TexCoord[0].s + s_delta, 1.0);
    float t_center = vsg_TexCoord[0].t;
    float delta_left_right = (s_right - s_left) * scale.x;
    float dz_left_right = (cstexture(displacementMap, vec2(s_right, t_center)).s - cstexture(displacementMap, vec2(s_left, t_center)).s) * scale.z;

    // TODO need to handle different origins of displacementMap vs diffuseMap etc,
    float t_delta = s_delta;
    float t_bottom = max(vsg_TexCoord[0].t - t_delta, 0.0);
    float t_top = min(vsg_TexCoord[0].t + t_delta, 1.0);
    float s_center = vsg_TexCoord[0].s;
    float delta_bottom_top = (t_top - t_bottom) * scale.y;
    float dz_bottom_top = (cstexture(displacementMap, vec2(s_center, t_top)).s - cstexture(displacementMap, vec2(s_center, t_bottom)).s) * scale.z;

    vec3 dx = normalize(vec3(delta_left_right, 0.0, dz_left_right));
    vec3 dy = normalize(vec3(0.0, delta_bottom_top, -dz_bottom_top));
    vec3 dz = normalize(cross(dx, dy));
    newNormal = normalize(vec3(dx * normal.x + dy * normal.y + dz * normal.z));
#endif
}

void main()
{
    vec4 vertex = vec4(vsg_Vertex, 1.0);
    vec3 normal = vsg_Normal;
    displaceGeometry(vsg_Vertex, vsg_Normal, vertex.xyz, normal);

#ifdef VSGCS_INSTANCES
   mat4x3 instanceMat = transpose(vsgcs_InstanceMat);
   mat3 instanceNormalMat = inverse(mat3(vsgcs_InstanceMat));
   vertex.xyz = instanceMat * vertex;
   normal = instanceNormalMat * normal;
#endif

    gl_Position = (pc.projection * pc.modelView) * vertex;

    eyePos = (pc.modelView * vertex).xyz;

    viewDir = - (pc.modelView * vertex).xyz;
#ifdef VSGCS_BILLBOARD_NORMAL
    normalDir = eyePos;         // -viewDir
#else
    mat3 normalMat = inverse(transpose(mat3(pc.modelView)));
    normalDir = (normalMat * normal);
#endif
    vertexColor = vsg_Color;
    for (int i = 0; i < 4; i++)
    {
        texCoord[i] = vsg_TexCoord[i];
    }
}
