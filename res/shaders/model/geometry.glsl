// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////// Primitives //
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// ////////////////////////////////////////////////////////////// Inputs //
in vec2 texCoordG[3];
in vec3 fragPos[3];

// ///////////////////////////////////////////////////////////// Outputs //
out vec2 texCoordF;
out vec3 normal;
out vec3 fPos;

// //////////////////////////////////////////////////////////////// Main //
void main() {
    vec3 a = fragPos[0];//gl_in[0].gl_Position.xyz;
    vec3 b = fragPos[1];//gl_in[1].gl_Position.xyz;
    vec3 c = fragPos[2];//gl_in[2].gl_Position.xyz;
    normal = normalize(cross(b - a, c - a));

    for (int i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position;
        texCoordF = texCoordG[i];
        fPos = fragPos[i];
        EmitVertex();
    }
    EndPrimitive();
}

// ///////////////////////////////////////////////////////////////////// //
