#include "Texture3D.h"

Texture3D::Texture3D(int format, int width, int height, int depth) {
	glGenTextures(1, &this->ID);

	this->bind();
	glTexImage3D(GL_TEXTURE_3D, 0, format, width, height, depth, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	this->unbind();
}

void Texture3D::bind(GLenum texture) {
	this->texture = texture;
	glActiveTexture(this->texture);
	glBindTexture(GL_TEXTURE_3D, this->ID);
}

void Texture3D::unbind() {
	if (!this->texture) {
		glActiveTexture(this->texture);
		this->texture = 0;
		glBindTexture(GL_TEXTURE_3D, 0);
	}
}

unsigned int Texture3D::getID() const{
	return this->ID;
}