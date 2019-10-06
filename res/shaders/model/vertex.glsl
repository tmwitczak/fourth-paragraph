// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////////// Inputs //
layout (location = 0) in vec3 posV;
layout (location = 1) in vec2 texCoordV;

// ///////////////////////////////////////////////////////////// Outputs //
out vec2 texCoordG;

// //////////////////////////////////////////////////////////// Uniforms //
uniform mat4 transform;

// //////////////////////////////////////////////////////////////// Main //
void main() {
    gl_Position = transform * vec4(posV, 1.0);
    texCoordG = texCoordV;
}

// ///////////////////////////////////////////////////////////////////// //
