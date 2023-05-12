#version 400 core

in vec3 vTexel;
in vec3 vFragGlobalPosition;

out vec4 fColor;

uniform sampler3D uField;

uniform vec3 uCameraPosition;
uniform vec3 uCameraFront;

uniform float z;

uniform vec3 uBoxVector;

uniform vec3 uGridDim;



vec3 globalToTexel(vec3 coord);

bool isTexelInBounds(vec3 texel);

void main() {
	float opacity = 0;

	float step_size = 1/uGridDim.x;

	vec3 density_sample = vFragGlobalPosition;
	while (isTexelInBounds(globalToTexel(density_sample))) {
		opacity += texture(uField, globalToTexel(density_sample)).x;

		density_sample = density_sample + step_size*uCameraFront;
	}

	fColor = vec4(vec3(1), opacity);
}

vec3 globalToTexel(vec3 coord) {
	return coord/uBoxVector+vec3(0.5);
}

bool isTexelInBounds(vec3 texel) {
	return (texel.x >= 0 && texel.x <= 1) && (texel.y >= 0 && texel.y <= 1) && (texel.z >= 0 && texel.z <= 1);
}