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
	bool init = false;
	GLBuffer();
	~GLBuffer();
	void EnableVAO();
	void DisableVAO();
	void* RetractVBOData(std::size_t vbo_indice);
	void SetVBOData(const std::vector<float>& vertex_array, void* ptr, int mod_pos);
	void SetVBOData(std::vector<float> vertex_array);
	void SetEBOData(std::vector<unsigned int> indice_array);
	void SetEBODataSize(std::size_t n);
	std::size_t GetEBODataSize();
	void AllocateVBOMemo(std::size_t buffer_indice, std::size_t buffer_size);
	void AllocateVBOMemo(int location, int tuple_size, unsigned long long buffer_size, unsigned long long offset);
	void FinishInitialization();
private:
	unsigned int VAO = 0, EBO = 0;
	std::size_t ebo_size = 0;
	std::vector<std::shared_ptr<unsigned int>> VBOs;
	std::vector<std::size_t> buffer_sizes;
};

#endif // !GL_BUFFERS_H
