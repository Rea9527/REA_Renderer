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

vec3 ads( ) {
    vec3 s = normalize( vec3(-Light.Direction));
    vec3 v = normalize(vec3(-Position));
    vec3 h = normalize( v + s );

    return
        Light.Intensity * (Material.Ka +
						  Material.Kd * max( dot(s, Normal), 0.0 ) +
						 Material. Ks * pow( max( dot(h, Normal), 0.0 ), Material.Shininess ) );
}

void main() {
    FragColor = vec4(ads(), 1.0);
}
