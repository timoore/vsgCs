#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#pragma import_defines (VSG_DIFFUSE_MAP, VSG_GREYSACLE_DIFFUSE_MAP, VSG_EMISSIVE_MAP, VSG_LIGHTMAP_MAP, VSG_NORMAL_MAP, VSG_METALLROUGHNESS_MAP, VSG_SPECULAR_MAP, VSGCS_OVERLAY_MAPS, VSG_TWO_SIDED_LIGHTING, VSG_WORKFLOW_SPECGLOSS, VSGCS_FLAT_SHADING, SHADOWMAP_DEBUG, VSGCS_TILE)

#include "descriptor_defs.glsl"

const float PI = 3.14159265359;
const float RECIPROCAL_PI = 0.31830988618;
const float RECIPROCAL_PI2 = 0.15915494;
const float EPSILON = 1e-6;
const float c_MinRoughness = 0.04;

#ifdef VSG_DIFFUSE_MAP
layout(set = PRIMITIVE_DESCRIPTOR_SET, binding = 0) uniform sampler2D diffuseMap;
#endif

#ifdef VSG_METALLROUGHNESS_MAP
layout(set = PRIMITIVE_DESCRIPTOR_SET, binding = 1) uniform sampler2D mrMap;
#endif

#ifdef VSG_NORMAL_MAP
layout(set = PRIMITIVE_DESCRIPTOR_SET, binding = 2) uniform sampler2D normalMap;
#endif

#ifdef VSG_LIGHTMAP_MAP
layout(set = PRIMITIVE_DESCRIPTOR_SET, binding = 3) uniform sampler2D aoMap;
#endif

#ifdef VSG_EMISSIVE_MAP
layout(set = PRIMITIVE_DESCRIPTOR_SET, binding = 4) uniform sampler2D emissiveMap;
#endif

#ifdef VSG_SPECULAR_MAP
layout(set = PRIMITIVE_DESCRIPTOR_SET, binding = 5) uniform sampler2D specularMap;
#endif

#ifdef VSGCS_TILE

layout(set = WORLD_DESCRIPTOR_SET, binding = 0) uniform sampler2D blueNoise;

// The params block should be sized with maxOverlays, but it's provoking a bug linking the shader stages
layout(set = TILE_DESCRIPTOR_SET, binding = 0) uniform TileParams 
{
  float geometricError;
  float maxPointSize;
  float fadeValue;
  float fadeOut;                // using a float as a bool
  OverlayParamBlock params[4];
} tileParams;

layout(set = TILE_DESCRIPTOR_SET, binding = 1) uniform sampler2D overlayTextures[maxOverlays];
#endif

layout(set = PRIMITIVE_DESCRIPTOR_SET, binding = 10) uniform PbrData
{
    vec4 baseColorFactor;
    vec4 emissiveFactor;
    vec4 diffuseFactor;
    vec4 specularFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaMask;
    float alphaMaskCutoff;
} pbr;

layout(set = VIEW_DESCRIPTOR_SET, binding = 0) uniform LightData
{
    vec4 values[2048];
} lightData;

layout(set = VIEW_DESCRIPTOR_SET, binding = 2) uniform sampler2DArrayShadow shadowMaps;

layout(location = 0) in vec3 eyePos;
// normalDir and viewDir are not normalized on input
#ifdef VSGCS_FLAT_SHADING
layout(location = 1) flat in vec3 normalDir;
#else
layout(location = 1) in vec3 normalDir;
#endif
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec3 viewDir;
layout(location = 4) in vec2 texCoord[4];

layout(location = 0) out vec4 outColor;

#ifdef VSGCS_OVERLAY_MAPS
vec4 overlayTexture(uint overlayNum)
{
    vec2 coords = texCoord[tileParams.params[overlayNum].coordIndex + 2];
    coords = coords * tileParams.params[overlayNum].scale;
    coords = coords + tileParams.params[overlayNum].translation;
    coords.t = 1.0 - coords.t;
    return texture(overlayTextures[overlayNum], coords);
}
#endif

vec2 noiseCoords()
{
    return vec2(gl_FragCoord.x / 256.0, gl_FragCoord.y / 256.0);
}

// If we think of the BRDF as a kind of shader node à la Blender or MaterialX, then this block holds
// the node parameters calculated from inputs. The other function parameters to the BRDF like light
// direction, etc. are different for each light or sample.
//
// A future version of this node might have constant properties of some kind.
struct PBRNode
{
    float perceptualRoughness;
    float metallic;
    vec3 specularEnvironmentR0;
    vec3 specularEnvironmentR90;
    float alphaRoughness;
    vec3 diffuseColor;
    vec3 specularColor;
};

PBRNode makePBRNode(float perceptualRoughness, float metallic, vec4 baseColor)
{
    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
    diffuseColor *= 1.0 - metallic;
    float alphaRoughness = perceptualRoughness * perceptualRoughness;
    vec3 specularColor = mix(f0, baseColor.rgb, metallic);
    // Compute reflectance.
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
    // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = specularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;
    return PBRNode(perceptualRoughness, metallic, specularEnvironmentR0, specularEnvironmentR90,
                   alphaRoughness, diffuseColor, specularColor);
}

float convertMetallic(vec3 diffuse, vec3 specular, float maxSpecular)
{
    float perceivedDiffuse = sqrt(0.299 * diffuse.r * diffuse.r + 0.587 * diffuse.g * diffuse.g + 0.114 * diffuse.b * diffuse.b);
    float perceivedSpecular = sqrt(0.299 * specular.r * specular.r + 0.587 * specular.g * specular.g + 0.114 * specular.b * specular.b);

    if (perceivedSpecular < c_MinRoughness)
    {
        return 0.0;
    }

    float a = c_MinRoughness;
    float b = perceivedDiffuse * (1.0 - maxSpecular) / (1.0 - c_MinRoughness) + perceivedSpecular - 2.0 * c_MinRoughness;
    float c = c_MinRoughness - perceivedSpecular;
    float D = max(b * b - 4.0 * a * c, 0.0);
    return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

// Construct PBRNode from SPECGLOSS parameters
PBRNode makePBRNode(vec4 diffuse, vec4 specular)
{
    vec3 f0 = vec3(0.04);
    float perceptualRoughness = 1.0 - specular.a;
    float reflectance = max(max(specular.r, specular.g), specular.b);
    float metallic = convertMetallic(diffuse.rgb, specular.rgb, reflectance);
    const float epsilon = 1e-6;
    vec3 baseColorDiffusePart = diffuse.rgb * ((1.0 - reflectance) / (1 - c_MinRoughness) / max(1 - metallic, epsilon)) * pbr.diffuseFactor.rgb;
    vec3 baseColorSpecularPart = specular.rgb - (vec3(c_MinRoughness) * (1 - metallic) * (1 / max(metallic, epsilon))) * pbr.specularFactor.rgb;
    vec4 baseColor = vec4(mix(baseColorDiffusePart, baseColorSpecularPart, metallic * metallic), diffuse.a);
    vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
    diffuseColor *= 1.0 - metallic;
    // Should we be using the input specular color instead?
    vec3 specularColor = mix(f0, baseColor.rgb, metallic);
    float alphaRoughness = perceptualRoughness * perceptualRoughness;
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = specularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;
    return PBRNode(perceptualRoughness, metallic, specularEnvironmentR0, specularEnvironmentR90,
                   alphaRoughness, diffuseColor, specularColor);
}

// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms, outlined in the Readme.MD Appendix.
struct PBRInfo
{
    float NdotL;                  // cos angle between normal and light direction
    float NdotV;                  // cos angle between normal and view direction
    float NdotH;                  // cos angle between normal and half vector
    float LdotH;                  // cos angle between light direction and half vector
    float VdotH;                  // cos angle between view direction and half vector
    float VdotL;                  // cos angle between view direction and light direction
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    float metalness;              // metallic value at the surface
    vec3 reflectance0;            // full reflectance color (normal incidence angle)
    vec3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 diffuseColor;            // color contribution from diffuse lighting
    vec3 specularColor;           // color contribution from specular lighting
};

// unused
vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    vec3 linOut = pow(srgbIn.xyz, vec3(2.2));
    return vec4(linOut,srgbIn.w);
}
// unused
vec4 LINEARtoSRGB(vec4 srgbIn)
{
    vec3 linOut = pow(srgbIn.xyz, vec3(1.0 / 2.2));
    return vec4(linOut, srgbIn.w);
}

float rcp(const in float value)
{
    return 1.0 / value;
}

float pow5(const in float value)
{
    return value * value * value * value * value;
}

// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal()
{
    vec3 result;
#ifdef VSG_NORMAL_MAP
    // Perturb normal, see http://www.thetenthplanet.de/archives/1180
    vec3 tangentNormal = texture(normalMap, texCoord[0]).xyz * 2.0 - 1.0;

    //tangentNormal *= vec3(2,2,1);
 
    vec3 q1 = dFdx(eyePos);
    vec3 q2 = dFdy(eyePos);
    vec2 st1 = dFdx(texCoord[0]);
    vec2 st2 = dFdy(texCoord[0]);

    vec3 N = normalize(normalDir);
    vec3 T = normalize(q1 * st2.t - q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    result = normalize(TBN * tangentNormal);
#else
    result = normalize(normalDir);
#endif
#ifdef VSG_TWO_SIDED_LIGHTING
    if (!gl_FrontFacing)
        result = -result;
#endif
    return result;
}

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
// Keeping this because we might use it instead of BRDF_Diffuse_Disney. If we do, the RECIPROCAL_PI
// will have to go.
vec3 BRDF_Diffuse_Lambert(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor * RECIPROCAL_PI;
}

vec3 BRDF_Diffuse_Disney(PBRInfo pbrInputs)
{
    float Fd90 = 0.5 + 2.0 * pbrInputs.perceptualRoughness * pbrInputs.LdotH * pbrInputs.LdotH;
    vec3 f0 = vec3(0.1);
    vec3 invF0 = vec3(1.0, 1.0, 1.0) - f0;
    float dim = min(invF0.r, min(invF0.g, invF0.b));
    float result = ((1.0 + (Fd90 - 1.0) * pow(1.0 - pbrInputs.NdotL, 5.0 )) * (1.0 + (Fd90 - 1.0) * pow(1.0 - pbrInputs.NdotV, 5.0 ))) * dim;
    return pbrInputs.diffuseColor * result;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs)
{
    //return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance90*pbrInputs.reflectance0) * exp2((-5.55473 * pbrInputs.VdotH - 6.98316) * pbrInputs.VdotH);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r + (1.0 - r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r + (1.0 - r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    // If light intensity isn't scaled by PI, then I don't think it should be a divisor here.
    return roughnessSq / (f * f);
}

vec3 BRDF(vec3 u_LightColor, vec3 v, vec3 n, vec3 l, vec3 h, PBRNode pbrNode, float ao)
{
    float unclmapped_NdotL = dot(n, l);
    vec3 reflection = -normalize(reflect(v, n));
    reflection.y *= -1.0f;

    float NdotL = clamp(unclmapped_NdotL, 0.001, 1.0);
    float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);
    float VdotL = clamp(dot(v, l), 0.0, 1.0);

    PBRInfo pbrInputs = PBRInfo(NdotL,
                                NdotV,
                                NdotH,
                                LdotH,
                                VdotH,
                                VdotL,
                                pbrNode.perceptualRoughness,
                                pbrNode.metallic,
                                pbrNode.specularEnvironmentR0,
                                pbrNode.specularEnvironmentR90,
                                pbrNode.alphaRoughness,
                                pbrNode.diffuseColor,
                                pbrNode.specularColor);

    // Calculate the shading terms for the microfacet specular shading model
    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    // Calculation of analytical lighting contribution
    vec3 diffuseContrib = (1.0 - F) * BRDF_Diffuse_Disney(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
    // Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
    vec3 color = NdotL * u_LightColor * (diffuseContrib + specContrib);

    color *= ao;

#ifdef VSG_EMISSIVE_MAP
    vec3 emissive = texture(emissiveMap, texCoord[0]).rgb * pbr.emissiveFactor.rgb;
#else
    vec3 emissive = pbr.emissiveFactor.rgb;
#endif
    color += emissive;

    return color;
}

void main()
{
    float brightnessCutoff = 0.001;

    float perceptualRoughness = 0.0;
    float metallic;
    vec3 diffuseColor;
    vec4 baseColor;
    float ambientOcclusion = 1.0;

#ifdef VSG_DIFFUSE_MAP
#ifdef VSG_GREYSACLE_DIFFUSE_MAP
    float v = texture(diffuseMap, texCoord[0].st).s * pbr.baseColorFactor;
    baseColor = vertexColor * vec4(v, v, v, 1.0);
#else
    baseColor = vertexColor * texture(diffuseMap, texCoord[0]) * pbr.baseColorFactor;
#endif
#else
    baseColor = vertexColor * pbr.baseColorFactor;
#endif

    // Blend overlays with the diffuse color. With these blend equations, an overlay can't make the
    // result transparent; is that correct?
#ifdef VSGCS_OVERLAY_MAPS
    for (int i = maxOverlays - 1; i >= 0; --i)
    {
        if (tileParams.params[i].enabled != 0)
        {
            vec4 overlayColor = overlayTexture(i);
            float overlayAlpha = tileParams.params[i].alpha * overlayColor.a;

            baseColor.rgb = overlayColor.rgb * overlayAlpha + baseColor.rgb * (1.0 - overlayAlpha);
            baseColor.a = overlayAlpha  + baseColor.a * (1.0 - overlayAlpha);
        }
    }
#endif
    if (pbr.alphaMask == 1.0f)
    {
        if (baseColor.a < pbr.alphaMaskCutoff)
            discard;
    }
#ifdef VSG_WORKFLOW_SPECGLOSS
    #ifdef VSG_DIFFUSE_MAP
    // XXX probably not correct for vsgCs with overlays
        vec4 diffuse = texture(diffuseMap, texCoord[0]);
    #else
        vec4 diffuse = vec4(1.0);
    #endif

    #ifdef VSG_SPECULAR_MAP
        vec4 specular = texture(specularMap, texCoord[0]).rgb;
    #else
        vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
    #endif
        PBRNode pbrNode = makePBRNode(diffuse, specular);
#else
        perceptualRoughness = pbr.roughnessFactor;
        metallic = pbr.metallicFactor;

    #ifdef VSG_METALLROUGHNESS_MAP
        vec4 mrSample = texture(mrMap, texCoord[0]);
        perceptualRoughness = mrSample.g * perceptualRoughness;
        metallic = mrSample.b * metallic;
    #endif
        PBRNode pbrNode = makePBRNode(perceptualRoughness, metallic, baseColor);
#endif

#ifdef VSG_LIGHTMAP_MAP
    ambientOcclusion = texture(aoMap, texCoord[0]).r;
#endif

    vec3 n = getNormal();
    vec3 v = normalize(viewDir);    // Vector from surface point to camera

    vec3 color = vec3(0.0, 0.0, 0.0);

    vec4 lightNums = lightData.values[0];
    int numAmbientLights = int(lightNums[0]);
    int numDirectionalLights = int(lightNums[1]);
    int numPointLights = int(lightNums[2]);
    int numSpotLights = int(lightNums[3]);
    int index = 1;

    if (numAmbientLights>0)
    {
        // ambient lights
        for(int i = 0; i<numAmbientLights; ++i)
        {
            vec4 ambient_color = lightData.values[index++];
            color += (baseColor.rgb * ambient_color.rgb) * (ambient_color.a * ambientOcclusion);
        }
    }

    // index used to step through the shadowMaps array
    int shadowMapIndex = 0;

    if (numDirectionalLights>0)
    {
        // directional lights
        for(int i = 0; i<numDirectionalLights; ++i)
        {
            vec4 lightColor = lightData.values[index++];
            vec3 direction = -lightData.values[index++].xyz;
            vec4 shadowMapSettings = lightData.values[index++];

            float brightness = lightColor.a;

            // check shadow maps if required
            bool matched = false;
            while ((shadowMapSettings.r > 0.0 && brightness > brightnessCutoff) && !matched)
            {
                mat4 sm_matrix = mat4(lightData.values[index++],
                                      lightData.values[index++],
                                      lightData.values[index++],
                                      lightData.values[index++]);

                vec4 sm_tc = (sm_matrix) * vec4(eyePos, 1.0);

                if (sm_tc.x >= 0.0 && sm_tc.x <= 1.0 && sm_tc.y >= 0.0 && sm_tc.y <= 1.0 && sm_tc.z >= 0.0 /* && sm_tc.z <= 1.0*/)
                {
                    matched = true;

                    float coverage = texture(shadowMaps, vec4(sm_tc.st, shadowMapIndex, sm_tc.z)).r;
                    brightness *= (1.0-coverage);

#ifdef SHADOWMAP_DEBUG
                    if (shadowMapIndex==0) color = vec3(1.0, 0.0, 0.0);
                    else if (shadowMapIndex==1) color = vec3(0.0, 1.0, 0.0);
                    else if (shadowMapIndex==2) color = vec3(0.0, 0.0, 1.0);
                    else if (shadowMapIndex==3) color = vec3(1.0, 1.0, 0.0);
                    else if (shadowMapIndex==4) color = vec3(0.0, 1.0, 1.0);
                    else color = vec3(1.0, 1.0, 1.0);
#endif
                }

                ++shadowMapIndex;
                shadowMapSettings.r -= 1.0;
            }

            if (shadowMapSettings.r > 0.0)
            {
                // skip lightData and shadowMap entries for shadow maps that we haven't visited for this light
                // so subsequent light pointions are correct.
                index += 4 * int(shadowMapSettings.r);
                shadowMapIndex += int(shadowMapSettings.r);
            }

            // if light is too dim/shadowed to effect the rendering skip it
            if (brightness <= brightnessCutoff ) continue;

            vec3 l = direction;         // Vector from surface point to light
            vec3 h = normalize(l+v);    // Half vector between both l and v
            float scale = brightness;

            color.rgb += BRDF(lightColor.rgb * scale, v, n, l, h, pbrNode,
                              ambientOcclusion);
        }
    }

    if (numPointLights>0)
    {
        // point light
        for(int i = 0; i<numPointLights; ++i)
        {
            vec4 lightColor = lightData.values[index++];
            vec3 position = lightData.values[index++].xyz;
            vec3 delta = position - eyePos;
            float distance2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            vec3 direction = delta / sqrt(distance2);

            vec3 l = direction;         // Vector from surface point to light
            vec3 h = normalize(l+v);    // Half vector between both l and v
            float scale = lightColor.a / distance2;

            color.rgb += BRDF(lightColor.rgb * scale, v, n, l, h,
                              pbrNode,
                              ambientOcclusion);
        }
    }

    if (numSpotLights>0)
    {
        // spot light
        for(int i = 0; i<numSpotLights; ++i)
        {
            vec4 lightColor = lightData.values[index++];
            vec4 position_cosInnerAngle = lightData.values[index++];
            vec4 lightDirection_cosOuterAngle = lightData.values[index++];

            vec3 delta = position_cosInnerAngle.xyz - eyePos;
            float distance2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            vec3 direction = delta / sqrt(distance2);
            float dot_lightdirection = -dot(lightDirection_cosOuterAngle.xyz, direction);

            vec3 l = direction;        // Vector from surface point to light
            vec3 h = normalize(l+v);    // Half vector between both l and v
            float scale = (lightColor.a * smoothstep(lightDirection_cosOuterAngle.w, position_cosInnerAngle.w, dot_lightdirection)) / distance2;

            color.rgb += BRDF(lightColor.rgb * scale, v, n, l, h,
                              pbrNode,
                              ambientOcclusion);
        }
    }
    // Blend tiles based on blue noise dithering. At every pixel we choose to render either the fading
    // in or fading out tile.
    // fading in: render if fadePercentage >= noise -> discard if fadePercentage < noise
    // fading out: render if fadePercentage < noise -> discard if fadePercentage >= noise
    // "fading out" is indicated by a negative fade percentage
    //
    // Ack, fade Cesium percentage is 0-1 fade in, 0-1 fade out!
#ifdef VSGCS_TILE
    float fadeCutoff = texture(blueNoise, noiseCoords()).r;
    if (tileParams.fadeOut == 0.0)
    {
        if (tileParams.fadeValue == 0.0 || tileParams.fadeValue < fadeCutoff)
            discard;
    }
    else
    {
        if (tileParams.fadeValue == 1.0 || tileParams.fadeValue >= fadeCutoff)
            discard;
    }
#endif
    outColor = vec4(color, baseColor.a);
}
