#version 430

layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;
layout(location=2) in vec2 VertexTexCoord;

out vec3 Position;
out vec3 Normal;
out vec2 TexCoord;
out vec4 ShadowCoord;

uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform mat3 NormalMatrix;
uniform mat4 ShadowMatrix;


void main() {

	Position = vec3(ModelViewMatrix * vec4(VertexPosition, 1.0f));
	Normal = normalize(NormalMatrix * VertexNormal);
	ShadowCoord = ShadowMatrix * vec4(VertexPosition, 1.0f);
	TexCoord = VertexTexCoord;
	gl_Position = MVP * vec4(VertexPosition, 1.0f);

}