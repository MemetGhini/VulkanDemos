#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (binding  = 1) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuse = texture(diffuseMap, inUV);
    diffuse.xyz *= 0.25 * inColor.xyz * inColor.w;
    outFragColor = diffuse ;
}