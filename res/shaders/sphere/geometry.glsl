// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////// Primitives //
layout (points) in;
layout (triangle_strip, max_vertices = 160) out;

// ///////////////////////////////////////////////////////////// Outputs //
out vec2 texCoordF;

// /////////////////////////////////////////////////////////// Uniforms //
uniform mat4 transform;
uniform int subdivisionLevelHorizontal;
uniform int subdivisionLevelVertical;

// /////////////////////////////////////////////////////////// Constants //
const float PI = 3.1415926535897932384626433832795;
const float RADIUS = 1.0;
const int SUBDIVISION_LEVEL_VERTICAL_MAX = 9;
const int SUBDIVISION_LEVEL_HORIZONTAL_MAX = 10;

// ////////////////////////////////////////////////////////// Conversion //
vec3 sphericalToCartesian(vec3 spherical) {
    const float r = spherical.x,
                theta = spherical.y,
                phi = spherical.z;
    return r * vec3(cos(theta) * cos(phi),
                    cos(theta) * sin(phi),
                    sin(theta));
}

// //////////////////////////////////////////////////////////////// Main //
void main() {
    // Subdivision levels
    float v = subdivisionLevelVertical;
    float h = subdivisionLevelHorizontal;

    // Spherical coordinates
    float r = RADIUS;
    float theta;
    float phi;

    // Sphere vertices
    vec4 sphere[SUBDIVISION_LEVEL_VERTICAL_MAX]
               [SUBDIVISION_LEVEL_HORIZONTAL_MAX];

    // Generate and draw a sphere for every input point
    for (int point = 0; point < gl_in.length(); point++) {
        vec4 origin = gl_in[point].gl_Position;

        // Generate vertices
        for (int i = 0; i <= v; i++) {
            theta = (PI / 2.0) - (PI * (i / v));

            for (int j = 0; j <= h; j++) {
                phi = (2.0 * PI) * (j / h);

                sphere[i][j] = transform
                    * (origin + vec4(
                        sphericalToCartesian(vec3(r, theta, phi)),
                        0.0));// - vec4(0.0, r / 2.0, 0.0, 0.0));
            }
        }

        // Draw the sphere
        for (int i = 0; i <= (v - 1); i++) {
            for (int j = 0; j <= h; j++) {
                gl_Position = sphere[i][j % int(h)];
                texCoordF = vec2(float(i) / float(v),
                                 float(j) / float(h));
                EmitVertex();

                gl_Position = sphere[i + 1][j % int(h)];
                texCoordF = vec2(float(i + 1) / float(v),
                                 float(j) / float(h));
                EmitVertex();
            }
            EndPrimitive();
        }
    }
}

// ///////////////////////////////////////////////////////////////////// //
