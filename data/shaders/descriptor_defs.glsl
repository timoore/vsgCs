#ifndef _DESCRIPTOR_DEFS_H
#define _DESCRIPTOR_DEFS_H 1

#define VIEW_DESCRIPTOR_SET 0
#define WORLD_DESCRIPTOR_SET 1
#define TILE_DESCRIPTOR_SET 2
#define PRIMITIVE_DESCRIPTOR_SET 3

layout(constant_id = 0) const int maxOverlays = 4;


struct OverlayParamBlock
{
  vec2 translation;
  vec2 scale;
  float alpha;
  uint enabled;
  uint coordIndex;
  // 4 bytes padding
};

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
