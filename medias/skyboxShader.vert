#version 430

layout (location = 0) in vec3 VertexPosition;

out vec3 TexCoords;


uniform mat4 MVP;

void main() {
    //Normal = normalize( NormalMatrix * VertexNormal);
    //Position = vec3( ModelViewMatrix * vec4(VertexPosition,1.0) );

	TexCoords = VertexPosition;

    gl_Position = MVP * vec4(VertexPosition,1.0).xyww;
}
