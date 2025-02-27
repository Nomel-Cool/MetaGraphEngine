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
	//void SetVBOData(const std::vector<float>& vertex_array, void* ptr, int mod_pos) = delete;
	//void SetVBOData(std::vector<float> vertex_array) = delete;
	template<typename T>
	void SetVBOData(const std::vector<T>& vertex_array, void* ptr, int mod_pos = 0)
	{
		if (!ptr)
			return;

		GLint buffer_size = 0;
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);

		size_t new_data_size = vertex_array.size() * sizeof(T);

		if (mod_pos + new_data_size > buffer_size)
		{
			printf("Error: Not enough space in VBO!\n");
			return;
		}

		memcpy(ptr, vertex_array.data(), new_data_size);

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	template<typename T>
	void SetVBOData(std::vector<T> vertex_array)
	{
		glBindVertexArray(VAO);
		auto sptr = std::make_shared<unsigned int>();
		glGenBuffers(1, &(*sptr));
		glBindBuffer(GL_ARRAY_BUFFER, *sptr);
		glBufferData(GL_ARRAY_BUFFER, vertex_array.size() * sizeof(T), &vertex_array[0], GL_DYNAMIC_DRAW);
		VBOs.emplace_back(sptr);
		buffer_sizes.emplace_back(vertex_array.size());
	}
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
