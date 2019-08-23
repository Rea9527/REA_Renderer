#version 430

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;

uniform struct LightInfo {
	vec4 Direction;
	vec3 Intensity;

} Light;

uniform struct MaterialInfo {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
} Material;


layout( location = 0 ) out vec4 FragColor;

layout (binding = 0) uniform sampler2D bgTex;
layout (binding = 1) uniform sampler2D rTex;
layout (binding = 2) uniform sampler2D gTex;
layout (binding = 3) uniform sampler2D bTex;
layout (binding = 4) uniform sampler2D blendMap;


vec4 blend() {
	vec4 blendMapColor = texture(blendMap, TexCoord);
    
    float backTextureAmount = 1 - (blendMapColor.r + blendMapColor.g + blendMapColor.b);
    vec2 tiledCoords = TexCoord * 40;
    vec4 bgTextureColor = texture(bgTex, tiledCoords) * backTextureAmount;
    vec4 rTextureColor = texture(rTex, tiledCoords) * blendMapColor.r;
    vec4 gTextureColor = texture(gTex, tiledCoords) * blendMapColor.g;
    vec4 bTextureColor = texture(bTex, tiledCoords) * blendMapColor.b;
    
    vec4 blendColor = bgTextureColor + rTextureColor + gTextureColor + bTextureColor;

	return blendColor;
}


vec3 ads( ) {
    vec3 s = normalize( vec3(-Light.Direction));
    vec3 v = normalize(vec3(-Position));
    vec3 h = normalize( v + s );

    return
        Light.Intensity * (Material.Ka +
						   Material.Kd * max( dot(s, Normal), 0.0 ) +
						   Material.Ks * pow( max( dot(h, Normal), 0.0 ), Material.Shininess ) );

}

void main() {
    FragColor = blend() * vec4(ads(), 1.0);
}
