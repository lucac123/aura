#version 400 core

in vec3 vTexel;
in vec3 vFragGlobalPosition;

out vec4 fColor;

uniform sampler3D uField;

uniform vec3 uCameraPosition;
uniform vec3 uCameraFront;

uniform float z;

uniform vec3 uBoxVector;
uniform vec3 uBoxLowerCorner;

uniform vec3 uGridDim;



vec3 globalToTexel(vec3 coord);

bool isTexelInBounds(vec3 texel);

void main() {
	vec3 front = normalize(vFragGlobalPosition - uCameraPosition);
	float ray_brightness = 0;

	float step_size = 0.001;
	float particle_brightness = 0.005;

	vec3 density_sample = vFragGlobalPosition;
	while (isTexelInBounds(globalToTexel(density_sample))) {
		ray_brightness += particle_brightness * texture(uField, globalToTexel(density_sample)).r;
		//ray_brightness += 0.01;
		density_sample = density_sample + step_size*front;
	}

	fColor = vec4(vec3(1), ray_brightness);
	//fColor = vec4(globalToTexel(vFragGlobalPosition), 1);
	//fColor = vec4(density_sample, ray_brightness);
	//fColor = vec4(vec3(ray_brightness), 1);
}

vec3 globalToTexel(vec3 coord) {
	return (coord - uBoxLowerCorner) / uBoxVector;
}

bool isTexelInBounds(vec3 texel) {
	return (texel.x >= 0 && texel.x <= 1) && (texel.y >= 0 && texel.y <= 1) && (texel.z >= 0 && texel.z <= 1);
}