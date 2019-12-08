// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////////// Inputs //
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in vec3 vTangent;

// ///////////////////////////////////////////////////////////// Outputs //
out vec3 gPosition;
out vec3 gNormal;
out vec2 gTexCoords;
out vec3 gTangent;

// //////////////////////////////////////////////////////////// Uniforms //
uniform mat4 world;
uniform mat4 transform;

uniform int instances;
uniform vec3 translate;

vec3 translations[25];
int index = 0;
void createTranslations() {
    for (float z = -16; z < 16; z += 6.4) {
        for (float x = -16; x < 16; x += 6.4) {
            vec3 translation;
            translation.x = float(x);// / 10.0f + offset;
//            translation.x = 0.0;
            translation.y = 0.0;
//            translation.z = 0.0;
            translation.z = float(z);// / 10.0f + offset;
            translations[index++] = translation + translate;
        }
    }
}

// //////////////////////////////////////////////////////////////// Main //
void main() {
    if (instances > 1) {
        createTranslations();
    }
    else {
        for (int i = 0; i < 25; i++) {
            translations[i] = vec3(0);
        }
    }

    gPosition = (world * vec4(vPosition + translations[gl_InstanceID], 1.0)).xyz;
    gNormal = normalize((world * vec4(vNormal, 1.0)).xyz);
    gTexCoords = vTexCoords;
    gTangent = normalize((world * vec4(vTangent, 1.0)).xyz);

    gl_Position = transform * vec4(vPosition + translations[gl_InstanceID], 1.0);
}

// ///////////////////////////////////////////////////////////////////// //
