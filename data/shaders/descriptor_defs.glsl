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

#endif
