#version 430

in vec3 Position;
in vec3 Normal;

uniform vec4 LightPosition;
uniform vec3 LightIntensity;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Shininess;

layout(location = 0) out vec4 FragColor;


vec3 ads() {

	vec3 s = normalize(vec3(LightPosition) - Position);
	vec3 v = normalize(vec3(-Position));
	vec3 h = normalize(v + s);

	return
	LightIntensity * (Ka +
					  Kd * (max(dot(s, Normal), 0.0)) +
					  Ks * pow(max(dot(h, Normal), 0.0), Shininess));
}

void main() {
	FragColor = vec4(ads(), 1.0);
	

}