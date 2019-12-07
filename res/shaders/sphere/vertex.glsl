// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////////// Inputs //
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

// ///////////////////////////////////////////////////////////// Outputs //
out vec3 gPosition;
out vec3 gNormal;
out vec2 gTexCoords;

// //////////////////////////////////////////////////////////// Uniforms //
uniform mat4 world;
uniform mat4 transform;

// //////////////////////////////////////////////////////////////// Main //
void main() {
    gPosition = (world * vec4(vPosition, 1.0)).xyz;
    gNormal = normalize((world * vec4(vNormal, 1.0)).xyz);
    gTexCoords = vTexCoords;

    gl_Position = transform * vec4(vPosition, 1.0);
}

// ///////////////////////////////////////////////////////////////////// //
