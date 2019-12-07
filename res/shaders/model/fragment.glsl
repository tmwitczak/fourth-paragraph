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

// Light parameters
struct LightParameters {
    float enable;

    vec3 direction;
    vec3 position;
    float angle;

    float ambientIntensity;
    vec3 ambientColor;
    float diffuseIntensity;
    vec3 diffuseColor;
    float specularIntensity;
    vec3 specularColor;
    float specularShininess;
};

uniform LightParameters lightDirectional;
uniform LightParameters lightPoint;
uniform LightParameters lightSpot1;
uniform LightParameters lightSpot2;

vec4 lambertBlinnPhong(LightParameters light,
                       vec3 lightDir,
                       float factor) {
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
    return factor * vec4(ambient + diffuse + specular, 1.0);
}

vec4 directional(LightParameters light) {
    return lambertBlinnPhong(light, -normalize(light.direction), 1.0);
}

vec4 point(LightParameters light) {
    vec3 lightDir = normalize(light.position - fPosition);
    return lambertBlinnPhong(light, lightDir, 1.0);
}

vec4 spot(LightParameters light) {
    vec3 lightDir = normalize(light.position - fPosition);
    float spotCosAngle = dot(lightDir, -normalize(light.direction));
    if (spotCosAngle < cos(light.angle)) {
        return vec4(0);
    }
    return lambertBlinnPhong(light, lightDir, (spotCosAngle - cos(light.angle)) / (1.0 - cos(light.angle)));
}

// //////////////////////////////////////////////////////////////// Main //
void main() {
    // Final pixel color
    outColor = (directional(lightDirectional) * lightDirectional.enable
                + point(lightPoint) * lightPoint.enable
                + spot(lightSpot1) * lightSpot1.enable
                + spot(lightSpot2) * lightSpot2.enable)
                * texture(texture0, fTexCoords);
}

// ///////////////////////////////////////////////////////////////////// //
