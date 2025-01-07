#include "GLBuffers.h"

GLBuffer::GLBuffer()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
}

GLBuffer::~GLBuffer()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void GLBuffer::EnableVAO()
{
	glBindVertexArray(VAO);
}

void GLBuffer::DisableVAO()
{
	glBindVertexArray(0);
}

void GLBuffer::SetVBOData(std::vector<float> vertex_array)
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertex_array.size() * sizeof(float), &vertex_array[0], GL_STATIC_DRAW);
}

void GLBuffer::SetEBOData(std::vector<unsigned int> indice_array)
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice_array.size() * sizeof(unsigned int), &indice_array[0], GL_STATIC_DRAW);
}

void GLBuffer::AllocateVBOMemo(int location, int tuple_size, unsigned long long buffer_size, unsigned long long offset)
{
	glVertexAttribPointer(location, tuple_size, GL_FLOAT, GL_FALSE, buffer_size, (void*)offset);
	glEnableVertexAttribArray(location);
}

void GLBuffer::FinishInitialization()
{
	glBindVertexArray(0); // 解绑 VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑 VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // 之后可以解绑 EBO
}
