#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#pragma import_defines (VSGCS_BILLBOARD_NORMAL, VSGCS_SIZE_TO_ERROR)

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

layout(set = VIEW_DESCRIPTOR_SET, binding = 1) uniform ViewportParams
{
    vec4 viewportExtent;     // x, y, width, height
#if 0
    ivec4 scissorExtent;     // offset x, offset y, width, height
    vec2 depthExtent;        // minDepth, maxDepth
#endif
} viewportParams;

#ifdef VSGCS_SIZE_TO_ERROR
// The params block should be sized with maxOverlays, but it's provoking a bug linking the shader stages
layout(set = TILE_DESCRIPTOR_SET, binding = 0) uniform TileParams 
{
  float geometricError;
  float maxPointSize;
  float fadeValue;
  float fadeOut;                // using a float as a bool
  OverlayParamBlock params[4];
} tileParams;
#endif

out gl_PerVertex{ vec4 gl_Position; float gl_PointSize; };


// VSG_BILLBOARD_NORMAL orients the normal to face the viewer.


void main()
{
    vec4 vertex = vec4(vsg_Vertex, 1.0);
    vec4 normal = vec4(vsg_Normal, 0.0);

    gl_Position = (pc.projection * pc.modelView) * vertex;
    gl_PointSize = 1.0;
    eyePos = (pc.modelView * vertex).xyz;
#ifdef VSGCS_SIZE_TO_ERROR
    vec4 displaced = vec4(eyePos, 1.0) + vec4(tileParams.geometricError, 0.0, 0.0, 0.0);
    vec4 screenDisplaced = pc.projection * displaced;
    float ndcRadius = screenDisplaced.x / screenDisplaced.w - gl_Position.x / gl_Position.w;
    float screenRadius = ndcRadius / 2.0 * viewportParams.viewportExtent.z;
    gl_PointSize = min(2.0 * screenRadius, tileParams.maxPointSize);
#endif
    viewDir = - (pc.modelView * vertex).xyz;
#ifdef VSGCS_BILLBOARD_NORMAL
    normalDir = viewDir;
#else
    normalDir = (pc.modelView * normal).xyz;
#endif
    vertexColor = vsg_Color;
    for (int i = 0; i < 4; i++)
    {
        texCoord[i] = vsg_TexCoord[i];
    }
}
