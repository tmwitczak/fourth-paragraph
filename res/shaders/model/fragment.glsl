// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////////// Inputs //
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec3 fTangent;

// ///////////////////////////////////////////////////////////// Outputs //
out vec4 outColor;

// //////////////////////////////////////////////////////////// Uniforms //
uniform bool pbrEnabled;

uniform vec3 viewPos;
uniform sampler2D texAo;
uniform sampler2D texAlbedo;
uniform sampler2D texMetalness;
uniform sampler2D texRoughness;
uniform sampler2D texNormal;
uniform mat4 vpMatrix;
uniform mat4 world;

// Lambert + Blinn-Phong

// Light parameters
struct LightParameters {
    float enable;

    vec3 direction;
    vec3 position;
    float angle;

    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;

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

vec3 CalcBumpedNormal()
{
    vec3 Normal = normalize(fNormal);
    vec3 Tangent = normalize(fTangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = cross(Tangent, Normal);
    vec3 BumpMapNormal = texture(texNormal, fTexCoords).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}

vec4 lambertBlinnPhong(LightParameters light,
                       vec3 lightDir,
                       float factor) {
    vec3 normal     = CalcBumpedNormal();
    // Ambient
    float ambientFactor = 1.0;
    vec3 ambient = ambientFactor
                   * light.ambientIntensity
                   * light.ambientColor;

    // Diffuse
    float diffuseFactor = clamp(dot(lightDir, normal), 0.0, 1.0);
    vec3 diffuse = diffuseFactor
                   * light.diffuseIntensity
                   * light.diffuseColor;

    // Specular
    float specularFactor = pow(
        clamp(
            dot(normal,
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

//#######################################################################
//#######################################################################

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a2     = pow(roughness, 4);

    float num   = a2;
    float denom = (pow(max(dot(N, H), 0.0), 2) * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


vec4 pbr(LightParameters light,
vec3 lightDir,
float factor) {
    vec3 albedo     = texture(texAlbedo, fTexCoords).rgb;
    albedo.r = pow(albedo.r, 2.2);
    albedo.g = pow(albedo.g, 2.2);
    albedo.b = pow(albedo.b, 2.2);
//    vec3 normal     = texture(texNormal, fTexCoords).rgb * 2.0 - 1.0;//fNormal;
//    vec3 normal     = fNormal;
    vec3 normal     = CalcBumpedNormal();
    float metallic  = texture(texMetalness, fTexCoords).r;
    float roughness = texture(texRoughness, fTexCoords).r;
    float ao        = texture(texAo, fTexCoords).r;

    vec3 N = normalize(normal);
    vec3 V = normalize(viewPos - fPosition);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // równanie odbicia
    vec3 Lo = vec3(0.0);

    // obliczy radiancję per-światło
    vec3 H = normalize(V + lightDir);
    vec3 radiance = light.diffuseColor * factor;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, lightDir, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, lightDir), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);

    // dodaj do wynikowej radiancji Lo
    float NdotL = max(dot(N, lightDir), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    return vec4(Lo, 1);
}
//#######################################################################
//#######################################################################

float attenuate(LightParameters light, float distance) {
    return 1.0 / (light.attenuationConstant
                    + light.attenuationLinear * distance
                    + light.attenuationQuadratic * pow(distance, 2));
}

vec4 directional(LightParameters light) {
    if (pbrEnabled) {
        return pbr(light, -normalize(light.direction), 1.0);
    }
    else {
            return lambertBlinnPhong(light, -normalize(light.direction), 1.0);
    }
}

vec4 point(LightParameters light) {
    vec3 lightDir = normalize(light.position - fPosition);
    if (pbrEnabled) {
        return pbr(light, lightDir, attenuate(light, length(light.position - fPosition)));
    }
    else {
        return lambertBlinnPhong(light, lightDir,
            attenuate(light, length(light.position - fPosition)));
    }
}

vec4 spot(LightParameters light) {
    vec3 lightDir = normalize(light.position - fPosition);
    float spotCosAngle = dot(lightDir, -normalize(light.direction));
    if (spotCosAngle < cos(light.angle)) {
        return vec4(0);
    }

    if (pbrEnabled) {
        return pbr(light, lightDir,
        (spotCosAngle - cos(light.angle)) / (1.0 - cos(light.angle))
        * attenuate(light, length(light.position - fPosition)));
    }
    else {
        return lambertBlinnPhong(light, lightDir,
            (spotCosAngle - cos(light.angle)) / (1.0 - cos(light.angle))
             * attenuate(light, length(light.position - fPosition)));
    }
}

// //////////////////////////////////////////////////////////////// Main //
void main() {
    // Final pixel color
    outColor = (directional(lightDirectional) * lightDirectional.enable
                + point(lightPoint) * lightPoint.enable
                + spot(lightSpot1) * lightSpot1.enable
                + spot(lightSpot2) * lightSpot2.enable);

    vec3 ambient = /*vec3(0.5) * texture(texAlbedo, fTexCoords).rgb * */texture(texAo, fTexCoords).rgb;
    vec3 color = ambient * outColor.rgb;

    if (pbrEnabled) {
//        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0/2.2));

        outColor = vec4(color, 1.0);
    }
    else {
        outColor = vec4(color, 1.0) * texture(texAlbedo, fTexCoords);
    }
}

// ///////////////////////////////////////////////////////////////////// //
