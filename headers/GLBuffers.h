#pragma once
#ifndef GL_BUFFERS_H
#define GL_BUFFERS_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>

class GLBuffer
{
public:
	GLBuffer();
	~GLBuffer();
	void EnableVAO();
	void DisableVAO();
	void SetVBOData(std::vector<float> vertex_array);
	void SetEBOData(std::vector<unsigned int> indice_array);
	void SetEBODataSize(std::size_t n);
	std::size_t GetEBODataSize();
	void AllocateVBOMemo(int location, int tuple_size, unsigned long long buffer_size, unsigned long long offset);
	void FinishInitialization();
private:
	unsigned int VAO = 0, EBO = 0;
	std::size_t ebo_size = 0;
	std::vector<std::shared_ptr<unsigned int>> VBOs;
};

#endif // !GL_BUFFERS_H
