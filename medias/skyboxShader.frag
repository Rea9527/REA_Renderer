#version 430

in vec3 TexCoords;

layout(location=0) out vec4 FragColor;

layout(binding=0) uniform samplerCube cubemap;

void main() {

	FragColor = texture(cubemap, TexCoords);
}