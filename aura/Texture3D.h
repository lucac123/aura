#pragma once
#include <glad/glad.h>

class Texture3D
{
private:
	unsigned int ID;
	GLenum texture;

public:
	Texture3D(int format, int width, int height, int depth);

	void bind(GLenum texture = GL_TEXTURE0);
	void unbind();

	unsigned int getID() const; //Returns ID corresponding with this texture
};

