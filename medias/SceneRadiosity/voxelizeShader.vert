#version 430 core

layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;

layout(location=0) out vec3 WorldPosition;
layout(location=1) out vec3 WorldNormal;


uniform mat4 ModelMatrix;
uniform mat4 MVP;


void main() {
    WorldPosition = vec3(ModelMatrix * vec4(VertexPosition, 1.0));
    WorldNormal = normalize(mat3(transpose(inverse(ModelMatrix))) * VertexNormal);
    
    gl_Position = MVP * vec4(VertexPosition, 1.0);
}

