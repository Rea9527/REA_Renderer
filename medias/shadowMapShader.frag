#version 430

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in vec4 ShadowCoord;

uniform struct LightInfo {
	vec4 Position;
	vec3 Intensity;

} Light;

uniform struct MaterialInfo {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;

} Material;

uniform sampler2DShadow shadowMap;

layout(location=0) out vec4 FragColor;

subroutine void RenderPassType();
subroutine uniform RenderPassType renderPass;


vec3 diffuseAndSpecPhongShading() {
	vec3 n = Normal;
	vec3 v = normalize(-Position);
	vec3 s = normalize(vec3(Light.Position) - Position);
	vec3 h = v + s;

	float sDotN = max(dot(s, n), 0.0f);
	vec3 diffuse = Material.Kd * sDotN;

	vec3 spec = vec3(0.0f);
	if (sDotN > 0.0) {
		spec = Material.Ks * pow(max(dot(h, n), 0.0f), Material.Shininess);
	}

	return Light.Intensity * (diffuse + spec);
}

layout(index=0) subroutine (RenderPassType)
void recordPass() {
	// depth is recorded to the depth map automatically
}

layout(index=1) subroutine (RenderPassType)
void shadowPass() {
	vec3 ambient = Light.Intensity * Material.Ka;
	vec3 diffAndSpec = diffuseAndSpecPhongShading();


	// ------------------Simple shadow mapping------------------
//	float shadow = 1.0;
//	if (ShadowCoord.z >= 0.0f ) {
//		shadow = textureProj(shadowMap, ShadowCoord);
//	}


	// ------------------Shadow with PCF------------------------
	float sum = 0.0;

	sum += textureProjOffset(shadowMap, ShadowCoord, ivec2(-1, -1));
	sum += textureProjOffset(shadowMap, ShadowCoord, ivec2(-1, 1));
	sum += textureProjOffset(shadowMap, ShadowCoord, ivec2(1, -1));
	sum += textureProjOffset(shadowMap, ShadowCoord, ivec2(1, 1));

	float shadow = sum * 0.25;

	FragColor = vec4(diffAndSpec * shadow + ambient, 1.0f);

	// Gamma correct
    FragColor = pow( FragColor, vec4(1.0 / 2.2) );

}


void main() {
	renderPass();
}