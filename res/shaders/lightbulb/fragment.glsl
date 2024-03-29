// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////////// Inputs //
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

// ///////////////////////////////////////////////////////////// Outputs //
out vec4 outColor;

// //////////////////////////////////////////////////////////// Uniforms //
uniform vec3 viewPos;

uniform sampler2D texAo;
uniform sampler2D texAlbedo;
uniform sampler2D texMetalness;
uniform sampler2D texRoughness;
uniform sampler2D texNormal;

uniform mat4 vpMatrix;
uniform mat4 world;


// //////////////////////////////////////////////////////////////// Main //
void main() {
    // Final pixel color
    outColor = pow((vec4(255, 147, 41, 1) / 255 + vec4(1.5)) * texture(texAlbedo, fTexCoords), vec4(1.6));
}

// ///////////////////////////////////////////////////////////////////// //
