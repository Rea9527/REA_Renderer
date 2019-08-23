#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout (location=0) in vec3 WorldPosition[];
layout (location=1) in vec3 WorldNormal[];

layout (location=0) out vec3 Position;
layout (location=1) out vec3 Normal;


void main() {
    vec3 edge1 = WorldPosition[1] - WorldPosition[0];
    vec3 edge2 = WorldPosition[2] - WorldPosition[0];
    vec3 faceNorm = abs(cross(edge1, edge2));

    for (uint i = 0; i < 3; i++) {
        Position = WorldPosition[i];
        Normal = WorldNormal[i];

        if (faceNorm.z > faceNorm.x && faceNorm.z > faceNorm.y)
            gl_Position = vec4(Position.x, Position.y, 0.0, 1.0);
        else if (faceNorm.y > faceNorm.x && faceNorm.y > faceNorm.z)
            gl_Position = vec4(Position.x, Position.z, 0.0, 1.0);
        else
            gl_Position = vec4(Position.y, Position.z, 0.0, 1.0);

        EmitVertex();
    }
    EmitPrimitive();
}
