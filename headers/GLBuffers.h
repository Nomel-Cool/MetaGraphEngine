#pragma once
#ifndef GL_BUFFERS_H
#define GL_BUFFERS_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

class GLBuffer
{
public:
	GLBuffer();
	~GLBuffer();
	void EnableVAO();
	void DisableVAO();
	void SetVBOData(std::vector<float> vertex_array);
	void SetEBOData(std::vector<unsigned int> indice_array);
	void AllocateVBOMemo(int location, int tuple_size, unsigned long long buffer_size, unsigned long long offset);
	void FinishInitialization();
private:
	unsigned int VAO = 0, VBO = 0, EBO = 0;
};

class GLBuffers
{
public:

private:

};

#endif // !GL_BUFFERS_H
