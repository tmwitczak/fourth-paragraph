// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////////// Inputs //
in vec2 texCoordF;
in vec3 normal;
in vec3 fPos;

// ///////////////////////////////////////////////////////////// Outputs //
out vec4 outColor;

// //////////////////////////////////////////////////////////// Uniforms //
uniform vec3 viewPos;
uniform sampler2D texture0;
uniform mat4 vpMatrix;
uniform mat4 world;

// //////////////////////////////////////////////////////////////// Main //
void main() {
    vec3 ambientIntensity = vec3(0.1);

    vec3 l = normalize(world * vec4(0, -80, 0, 1)).xyz;//normalize(vec3(0, 100, 0) - fPos);
//    vec3 l = normalize((world * vec4(0, 800, 0, 1)).xyz - fPos);
    vec3 n = -normal;
    vec3 c = vec3(1);
    float i = 1;

    vec3 diffuseColor = vec3(0);
    vec3 specularColor = vec3(0);

    float diffuseFactor = dot(l, n);

    if (diffuseFactor > 0) {
        diffuseColor = diffuseFactor * c * i;

        ambientIntensity = vec3(0);

        //    vec3 lightDir   = l;//normalize(lightPos - FragPos);
        vec3 v = -normalize(viewPos - fPos);
            vec3 h = normalize(l + v);
//        vec3 h = normalize(reflect(l, n));
        float shininess = 16;
        float specularFactor = dot(n, h);

        if (specularFactor > 0) {
            specularColor = c * pow(specularFactor, shininess);
        }
    }


    outColor = (vec4(ambientIntensity, 1) + vec4(diffuseColor, 1) + vec4(specularColor, 1)) * texture(texture0, texCoordF);
}

// ///////////////////////////////////////////////////////////////////// //
