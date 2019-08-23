#version 430

layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;
layout(location=2) in vec2 VertexTexCoord;
layout(location=3) in mat4 InstanceModelMat;

layout(location=0) out vec3 Position;
layout(location=1) out vec3 Normal;
layout(location=2) out vec2 TexCoord;


uniform mat4 ProjectionViewMatrix;
uniform mat4 ViewMatrix;



void main() {

	Position = vec3(ViewMatrix * InstanceModelMat * vec4(VertexPosition, 1.0f));
	Normal = normalize(mat3(transpose(inverse(ViewMatrix * InstanceModelMat))) * VertexNormal);
	gl_Position = ProjectionViewMatrix * InstanceModelMat * vec4(VertexPosition, 1.0f);

	TexCoord = VertexTexCoord;
}