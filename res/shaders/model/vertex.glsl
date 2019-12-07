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

uniform bool instantiate;

vec3 translations[100];
int index = 0;
float offset = 0.1f;
void createTranslations() {
    for (int z = -10; z < 10; z += 2)
    {
        for (int x = -10; x < 10; x += 2)
        {
            vec3 translation;
//            translation.x = float(x) / 10.0f + offset;
            translation.x = 0.0;
            translation.y = 0.0;
            translation.z = 0.0;
//            translation.z = float(z) / 10.0f + offset;
            translations[index++] = translation;
        }
    }
}

// //////////////////////////////////////////////////////////////// Main //
void main() {
    createTranslations();

    gPosition = (world * vec4(vPosition + 16 * translations[gl_InstanceID], 1.0)).xyz;
    gNormal = normalize((world * vec4(vNormal, 1.0)).xyz);
    gTexCoords = vTexCoords;

    gl_Position = transform * vec4(vPosition + 16 * translations[gl_InstanceID], 1.0);
}

// ///////////////////////////////////////////////////////////////////// //
