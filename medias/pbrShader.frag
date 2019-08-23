#version 430
// in
layout (location=0) in vec3 Position;
layout (location=1) in vec3 Normal;
layout (location=2) in vec2 TexCoord;
//out
layout (location=0) out vec4 FragColor;

uniform struct LightInfo {
	vec3 Position;
	vec3 Color;

} Light[4];

// parameters for reflectance equation
uniform vec3 Albedo;
uniform float Roughness;
uniform float Metallic;
uniform float AOFactor;
// camera position
uniform vec3 CamPos;

const float PI = 3.14159265359;


// Distribution Function for BRDF - using GGX
float distributionGGX(vec3 norm, vec3 halfVec, float roughness);
// Geometry function for BRDF - Smith method(GGX, view dir and light dir)
float geometrySchlickGGX(float cosVal, float roughness);
float geometrySmith(vec3 norm, vec3 viewDir, vec3 lightDir, float roughness);
// Fresnel Function for BRDF (Fresnel-Schlick)
vec3 fresnelSchlick(float cosVal, vec3 F0, float roughness);


void main() {
	
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(-Position);

	// F0 for Fresnel function
	vec3 F0 = vec3(0.4f);
	F0 = mix(F0, Albedo, Metallic);
	
	// Reflectance equation - sum up the contributions of four lights
	vec3 L0 = vec3(0.0f);
	for (int i = 0; i < 1; i++) {
		vec3 lightVec = Light[i].Position - Position;
		vec3 lightDir = normalize(lightVec);
		vec3 halfVec = normalize(viewDir + lightDir);

		// Cook-Torrance BRDF computation
		float NDF = distributionGGX(norm, halfVec, Roughness);
		float G = geometrySmith(norm, viewDir, lightDir, Roughness);
		vec3 Fr = fresnelSchlick(max(dot(halfVec, viewDir), 0.0f), F0, Roughness);

		// compute diffuse factor (Fresnel reflectance is approximated to specular ratio)
		vec3 Ks = Fr;
		vec3 Kd = 1 - Ks;
		Kd *= 1.0f - Metallic;

		// compute specular part of cook-torrance BRDF
		float NdotL = max(dot(norm, lightDir), 0.0f);
		vec3 DFG = NDF * G * Fr;
		float denominator = 4 * max(dot(norm, viewDir), 0.0f) * NdotL;
		vec3 specular = DFG / max(denominator, 0.001);

		// distance from light to point, for attenuation of light
		float dist = length(lightVec);
		float attenuation = 1.0f / (dist * dist);
		vec3 radiance = Light[i].Color * attenuation;

		// compute L0
		L0 += (Kd * Albedo / PI + specular) * radiance * NdotL;

	}

	// ambient 0.3
	vec3 ambient = vec3(0.3) * Albedo * AOFactor;
	vec3 color = ambient + L0;
	// HDR-LDR and gamma correct
	color = color / (color + vec3(1.0f));
	color = pow(color, vec3(1.0/2.2));

	FragColor = vec4(color, 1.0f);

}

// Trowbridge-Reitz GGX
float distributionGGX(vec3 norm, vec3 halfVec, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(norm, halfVec), 0.0f);
	
	float subExp = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
	float ret = a2 / (PI * subExp * subExp);

	return ret;
}

// Schilick-GGX
float geometrySchlickGGX(float cosVal, float roughness) {
	float r = roughness + 1.0f;
	float k = (r * r) / 8.0f;

	float ret = cosVal / ((1.0 - k) * cosVal + k);
	return ret;
}

float geometrySmith(vec3 norm, vec3 viewDir, vec3 lightDir, float roughness) {
	float ndotv = max(dot(norm, viewDir), 0.0f);
	float ndotl = max(dot(norm, lightDir), 0.0f);

	float g1 = geometrySchlickGGX(ndotv, roughness);
	float g2 = geometrySchlickGGX(ndotl, roughness);

	return g1 * g2;

}

vec3 fresnelSchlick(float cosVal, vec3 F0, float roughness) {
	return F0 + (1.0 - F0) * pow((1.0 - cosVal), 5.0);
}