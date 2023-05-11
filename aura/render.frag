#version 400 core

in vec3 vTexel;

out vec4 fColor;

void main() {
	fColor = vec4(vTexel.x);
}