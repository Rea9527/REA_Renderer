#version 430

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;


struct LightInfo {
	vec4 Position;
	vec3 Intensity;
};
uniform LightInfo Light[3];

struct LightMaterial {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
};
uniform LightMaterial Material;


layout(location=0) out vec4 FragColor;

// subroutine definition
subroutine vec4 BloomPassType();
subroutine uniform BloomPassType bloomPass;

// uniforms
uniform float LumThresh;
uniform float AvgLum;
uniform float Weights[5];
uniform float PixOffset[] = {0.0, 1.0, 2.0, 3.0, 4.0};

layout(binding=0) uniform sampler2D HdrTex;
layout(binding=1) uniform sampler2D BrightTex;
layout(binding=2) uniform sampler2D BlurTex;

// XYZ/RGB conversion matrices from:
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

uniform mat3 rgb2xyz = mat3( 
  0.4124564, 0.2126729, 0.0193339,
  0.3575761, 0.7151522, 0.1191920,
  0.1804375, 0.0721750, 0.9503041 );

uniform mat3 xyz2rgb = mat3(
  3.2404542, -0.9692660, 0.0556434,
  -1.5371385, 1.8760108, -0.2040259,
  -0.4985314, 0.0415560, 1.0572252 );

uniform float Exposure = 0.35;
uniform float White = 0.928;


// compute luminance of a single color
float luminance( vec3 color ) {
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

// ads
vec4 ads() {
	
	vec3 v = normalize(-Position);
	vec3 total = vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 3; i++) {
		vec3 s = normalize(vec3(Light[i].Position) - Position);
		vec3 h = normalize(s + v);

		total += Light[i].Intensity * 
						(Material.Ka +
						 Material.Kd * max(dot(s, Normal), 0.0f) + 
						 Material.Ks * pow(max(dot(h, Normal), 0.0f), Material.Shininess));
	}

	return vec4(total, 1.0f);
	
}

// First Pass - rendering basics
subroutine (BloomPassType)
vec4 renderPass() {
	
	return ads();
}

// Second Pass - extract the bright part
subroutine (BloomPassType)
vec4 brightPass() {
	vec4 val = texture(HdrTex, TexCoord);
    if( luminance(val.rgb) > LumThresh )
        return val;
    else
        return vec4(0.0f);
}

subroutine (BloomPassType)
vec4 verGaussPass() {
	float dy = 1.0f / (textureSize(BrightTex, 0)).y;

	vec4 sum = texture(BrightTex, TexCoord) * Weights[0];
	for (int i = 1; i < 5; i++) {
		sum += texture(BrightTex, TexCoord + vec2(0.0f, PixOffset[i]) * dy) * Weights[i];
		sum += texture(BrightTex, TexCoord - vec2(0.0f, PixOffset[i]) * dy) * Weights[i];
	}
	return sum;
}

subroutine (BloomPassType)
vec4 horGaussPass() {
	float dx = 1.0f / (textureSize(BlurTex, 0)).x;

	vec4 sum = texture(BlurTex, TexCoord) * Weights[0];
	for (int i = 1; i < 5; i++) {
		sum += texture(BlurTex, TexCoord + vec2(PixOffset[i], 0.0f) * dx) * Weights[i];
		sum += texture(BlurTex, TexCoord - vec2(PixOffset[i], 0.0f) * dx) * Weights[i];
	}
	return sum;
}

subroutine (BloomPassType)
vec4 tonePass() {
	vec4 color = texture(HdrTex, TexCoord);

	// convert to XYZ
	vec3 xyzColor = rgb2xyz * vec3(color);

	// convert to xyY
	float xyzSum = xyzColor.x + xyzColor.y + xyzColor.z;
	vec3 xyYColor = vec3(xyzColor.x / xyzSum, xyzColor.y / xyzSum, xyzColor.y);

	// tone mapping
	float L = (Exposure * xyYColor.z) / AvgLum;
    L = (L * ( 1 + L / (White * White) )) / ( 1 + L );

	// convert back to XYZ
	xyzColor.x = (L * xyYColor.x) / (xyYColor.y);
    xyzColor.y = L;
    xyzColor.z = (L * (1 - xyYColor.x - xyYColor.y))/xyYColor.y;

	// convet back to RGB
	vec4 toneMapColor = vec4( xyz2rgb * xyzColor, 1.0);

	// add the tonemap color and the blur color
	vec4 blurColor = texture(BrightTex, TexCoord);


	return toneMapColor + blurColor;
}


void main() {
	FragColor = bloomPass();

}