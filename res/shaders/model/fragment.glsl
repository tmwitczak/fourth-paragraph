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
    // Light parameters
    vec3 ambient = vec3(0.0);

    float diffuseFactor = 0.0;
    vec3 diffuse = vec3(0.0);

    float specularFactor = 0.0;
    vec3 specular = vec3(0.0);

    // Ambient
    ambient = vec3(0.1);

    // Diffuse
    vec3 l = normalize(world * vec4(80, -80, 0, 1)).xyz;//normalize(vec3(0, 100, 0) - fPos);
    //    vec3 l = normalize((world * vec4(0, 800, 0, 1)).xyz - fPos);
    vec3 n = -normal;
    vec3 c = vec3(1);
    float i = 1;


    diffuseFactor = dot(l, n);

    if (diffuseFactor > 0) {
        diffuse = diffuseFactor * c * i;

//        ambient = vec3(0);

        //    vec3 lightDir   = l;//normalize(lightPos - FragPos);
        vec3 v = -normalize(viewPos - fPos);
        vec3 h = normalize(l + v);
        //        vec3 h = normalize(reflect(l, n));
        float shininess = 64;
        float specularFactor = dot(n, h);

        if (specularFactor > 0) {
            specular = c * pow(specularFactor, shininess);
        }
    }


    outColor = vec4(ambient + diffuse + specular, 1) * texture(texture0, texCoordF);
}

// ///////////////////////////////////////////////////////////////////// //
