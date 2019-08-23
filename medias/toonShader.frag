#version 430

in vec3 Position;
in vec3 Normal;

uniform vec4 LightPosition;
uniform vec3 LightIntensity;

uniform vec3 Ka;
uniform vec3 Kd;

layout(location=0) out vec4 FragColor;

subroutine vec4 RenderPassType();
subroutine uniform RenderPassType renderPass;

// toon shading levels
const int levels = 3;
const float scalarfactor = 1.0f / levels;

// texture from pass1 for filtering
layout (binding = 0) uniform sampler2D renderTex;
// filtering threshold
uniform float edgeThreshold;


float luminance(vec3 color) {
	return dot(LightIntensity, color);
}

vec3 toonShading() {

	vec3 s = normalize( vec3(LightPosition) - Position);
	float cosine = max (dot(s, Normal), 0.0f);

	return LightIntensity * ( Ka +
							  Kd * floor(cosine * levels) * scalarfactor);

}

subroutine (RenderPassType)
vec4 filtering() {
	ivec2 p = ivec2(gl_FragCoord.xy);

	float s00 = luminance(
					texelFetchOffset(renderTex, p, 0, ivec2(-1, 1)).rgb );

	float s01 = luminance(
					texelFetchOffset(renderTex, p, 0, ivec2(0, 1)).rgb );

	float s02 = luminance(
					texelFetchOffset(renderTex, p, 0, ivec2(1, 1)).rgb );

	float s10 = luminance(
					texelFetchOffset(renderTex, p, 0, ivec2(-1, 0)).rgb );

	float s12 = luminance(
					texelFetchOffset(renderTex, p, 0, ivec2(1, 0)).rgb );

	float s20 = luminance(
					texelFetchOffset(renderTex, p, 0, ivec2(-1, -1)).rgb );

	float s21 = luminance(
					texelFetchOffset(renderTex, p, 0, ivec2(0, -1)).rgb );

	float s22 = luminance(
					texelFetchOffset(renderTex, p, 0, ivec2(1, -1)).rgb );
	
	float sx = - s00 + s02 - 2 * s10 + 2 * s12 - s20 + s22;
	float sy = - s00 - 2 * s01 - s02 + s20 + 2 * s21 + s22;

	float res = sx * sx + sy * sy;

	if (res > edgeThreshold) return vec4(toonShading(),1.0f) + vec4(0.5);
	else return vec4(0.0f, 0.0f, 0.0f, 1.0);

}

subroutine (RenderPassType)
vec4 shading() {
	return vec4(toonShading(), 1.0f);
}


void main() {
	
	FragColor = renderPass();

}
