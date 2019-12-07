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
uniform sampler2D texture0;
uniform mat4 vpMatrix;
uniform mat4 world;

// Lambert + Blinn-Phong
vec3 lightDir = -normalize(world * vec4(0, -1, 1, 1)).xyz;

// Light parameters
struct LightParameters {
    float ambientIntensity;
    vec3 ambientColor;
    float diffuseIntensity;
    vec3 diffuseColor;
    float specularIntensity;
    vec3 specularColor;
    float specularShininess;
};

uniform LightParameters test;

vec4 lambertBlinnPhong(LightParameters light) {
    // Ambient
    float ambientFactor = 1.0;
    vec3 ambient = ambientFactor
                   * light.ambientIntensity
                   * light.ambientColor;

    // Diffuse
    float diffuseFactor = clamp(dot(lightDir, fNormal), 0.0, 1.0);
    vec3 diffuse = diffuseFactor
                   * light.diffuseIntensity
                   * light.diffuseColor;

    // Specular
    float specularFactor = pow(
        clamp(
            dot(fNormal,
                normalize(lightDir +                        // Half
                          normalize(viewPos - fPosition))), // View
            0.0, 1.0),
        light.specularShininess
    );
    vec3 specular = specularFactor
                    * light.specularIntensity
                    * light.specularColor;

    // Final lighting
    return vec4(ambient + diffuse + specular, 1.0);
}

// //////////////////////////////////////////////////////////////// Main //
void main() {
    // Final pixel color
    outColor = lambertBlinnPhong(test)
               * texture(texture0, fTexCoords);
}

// ///////////////////////////////////////////////////////////////////// //
