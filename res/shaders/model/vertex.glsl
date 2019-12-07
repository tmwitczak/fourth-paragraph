// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////////// Inputs //
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

// ///////////////////////////////////////////////////////////// Outputs //
out vec2 texCoordG;
out vec3 gNormal;
out vec3 fragPos;

// //////////////////////////////////////////////////////////// Uniforms //
uniform mat4 world;
uniform mat4 transform;

vec3 translations[100];
int index = 0;
float offset = 0.1f;
void createTranslations() {
    for (int z = -10; z < 10; z += 2)
    {
        for (int x = -10; x < 10; x += 2)
        {
            vec3 translation;
            translation.x = float(x) / 10.0f + offset;
            translation.y = 0.0;
            translation.z = float(z) / 10.0f + offset;
            translations[index++] = translation;
        }
    }
}

// //////////////////////////////////////////////////////////////// Main //
void main() {
    createTranslations();
    gl_Position = transform * vec4(vPosition + 16 * translations[gl_InstanceID], 1.0);
//    gl_Position = transform * vec4(vPosition, 1.0);
    fragPos = (world * vec4(vPosition + 16 * translations[gl_InstanceID], 1.0)).xyz;
//    fragPos = (world * vec4(vPosition, 1.0)).xyz;
//    gNormal = normalize(world * vec4(vNormal, 1.0)).xyz + 8 * translations[gl_InstanceID];
    gNormal = normalize((world * vec4(vNormal, 1.0)).xyz);
    texCoordG = vTexCoords;
}

// ///////////////////////////////////////////////////////////////////// //
