#version 430

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;


uniform struct LightInfo {
	vec3 Position;
	vec3 Color;

	float Constant;
	float Linear;
	float Quadratic;

} Light;

uniform struct MaterialInfo {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
} Material;

// kernel for SSAO with kernel size 64
uniform vec3 ssaoKernel[64];
int kernelSize = 64;
// hemisphere radius
uniform float uRadius;
// blur filter size
uniform int blurSize = 4;
// Tile noise texture over screen based on screen size(800*600) and noise size(4*4)
const vec2 noiseScale = vec2(800.0 / 4.0, 600.0 / 4.0);

// projection matrix
uniform mat4 ProjectionMatrix;

subroutine void RenderPassType();
subroutine uniform RenderPassType renderPass;

layout(binding = 0) uniform sampler2D posTex;
layout(binding = 1) uniform sampler2D normTex;
layout(binding = 2) uniform sampler2D colorTex;
layout(binding = 3) uniform sampler2D ssaoBuf;
layout(binding = 4) uniform sampler2D ssaoBlurBuf;
layout(binding = 5) uniform sampler2D noiseTex;


layout( location = 0 ) out vec4 FragColor;
layout( location = 1 ) out vec3 gPos;
layout( location = 2 ) out vec3 gNormal;
layout( location = 3 ) out vec3 gColor;

vec3 ads(in vec3 pos, in vec3 norm, in vec3 color) {
    vec3 s = normalize( Light.Position - pos);
    vec3 v = normalize(vec3(-pos));
    vec3 h = normalize( v + s );

	vec3 diff = color * max( dot(s, norm), 0.0 );
	vec3 spec = Material.Ks == vec3(0.0f) ? vec3(0.0f) : Material.Ks * pow( max( dot(h, norm), 0.0 ), Material.Shininess);

	// compute attenuation for point light
	float dist = length(Light.Position - pos);
	float attenuation = 1.0f / (Light.Constant + Light.Linear * dist + Light.Quadratic * dist * dist);

	vec3 diffSpec = attenuation * (diff + spec);

    return
        Light.Color * (Material.Ka + diffSpec);
}

// Geometry pass to store the geometric information into the user-defined framebuffer
layout(index = 0) subroutine (RenderPassType)
void geometryPass() {
	gPos = Position;
	gNormal = Normal;
	gColor = Material.Kd;
}

// SSAO pass - get the ambient occulusion factor with kernel and random noise
layout(index = 1) subroutine (RenderPassType)
void ssaoPass() {
	// get relevant value from texture
	vec3 pos = texture(posTex, TexCoord).xyz; // view space position
	vec3 normal = normalize(texture(normTex, TexCoord).rgb); // view space normal
	vec3 noiseVec = normalize(texture(noiseTex, TexCoord * noiseScale).xyz); // view space? doesnt matter, it should be linear independent with normal!

	// compute tangent space TBN Matrix using Gramm-Schmidt process for transforming kernel from tangent space to view space
	vec3 tangent = normalize(noiseVec - normal * (dot(noiseVec, normal)));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal); // normal as the third column(z) because our kernel is orienting along the normal(z-axis)
	
	// compute the ambient occlusion factor
	float occlusion = 0.0f;
	for (int i = 0; i < kernelSize; i++) {
		vec3 sp = TBN * ssaoKernel[i]; // transform from tangent space to view space and the random noise now is applied to the kernel;
		sp = pos + sp * uRadius; // view space kernel

		// apply perpective projection, to projection space
		vec4 offset = vec4(sp, 1.0f);
		offset = ProjectionMatrix * offset; // to clip space
		offset.xyz /= offset.w; // perspective division
		offset.xyz = offset.xyz * 0.5 + 0.5; // to 0.0 - 1.0, now offset is in screen space

		// then we can get the depth from the position texture using the screen space offset
		float sampleDepth = texture(posTex, offset.xy).z;

		//float rangeCheck = abs(pos.z - sampleDepth) < uRadius ? 1.0 : 0.0;
		float rangeCheck = smoothstep(0.0f, 1.0f, uRadius / abs(pos.z - sampleDepth));
		occlusion += ((sampleDepth >= sp.z + 0.025) ? 1.0 : 0.0) * rangeCheck;
	
	}

	// normalize occlusion factor and invert
	occlusion = 1.0f - (occlusion / kernelSize);

	FragColor = vec4(occlusion, occlusion, occlusion, 1.0f);
}

// SSAO blur pass - blur the SSAO texture from the previous pass to remove the noise
layout(index = 2) subroutine (RenderPassType)
void ssaoBlurPass() {
	vec2 texelSize = 1.0f / textureSize(ssaoBuf, 0);
	// move the filter (because the for loops are started from 0)
	vec2 offset = vec2(-blurSize * 0.5 + 0.5);

	float occlusion = 0.0f;
	// blur filtering
	for (int i = 0; i < blurSize; i++) {
		for (int j = 0; j < blurSize; j++) {
			vec2 coord = TexCoord + (offset + vec2(float(i), float(j))) * texelSize;
			occlusion += texture(ssaoBuf, coord).r;
		}
	}
	occlusion = occlusion / (blurSize * blurSize);
	FragColor = vec4(vec3(occlusion), 1.0f);
}

// Final lighting pass
layout(index = 3) subroutine (RenderPassType)
void lightingPass() {
	vec3 pos = texture(posTex, TexCoord).rgb;
	vec3 norm = normalize(texture(normTex, TexCoord).rgb);
	vec3 color = texture(colorTex, TexCoord).rgb;

	float occlusion = texture(ssaoBlurBuf, TexCoord).r;

	vec3 ambient = Material.Ka * occlusion;

	vec3 s = normalize( Light.Position - pos);
    vec3 v = normalize(vec3(-pos));
    vec3 h = normalize( v + s );

	vec3 diff = color * max( dot(s, norm), 0.0 );
	vec3 spec = Material.Ks == vec3(0.0f) ? vec3(0.0f) : Material.Ks * pow( max( dot(h, norm), 0.0 ), Material.Shininess);

	// compute attenuation for point light
	float dist = length(Light.Position - pos);
	float attenuation = 1.0f / (Light.Constant + Light.Linear * dist + Light.Quadratic * dist * dist);

	vec3 diffSpec = attenuation * (diff + spec);

    FragColor =  vec4(Light.Color * (ambient + diffSpec), 1.0f);

}

void main() {

	renderPass();
}
