#include "GLBuffers.h"

GLBuffer::GLBuffer()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &EBO);
}

GLBuffer::~GLBuffer()
{
	glDeleteVertexArrays(1, &VAO);
	for (auto& sptr : VBOs)
	{
		glDeleteBuffers(1, &(*sptr));
		sptr.reset();
	}
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

void GLBuffer::SetVBOData(const std::vector<float>& vertex_array)
{
	glBindVertexArray(VAO);
	auto sptr = std::make_shared<unsigned int>();
	glGenBuffers(1, &(*sptr));
	glBindBuffer(GL_ARRAY_BUFFER, *sptr);
	VBOs.emplace_back(sptr);
	glBufferData(GL_ARRAY_BUFFER, vertex_array.size() * sizeof(float), &vertex_array[0], GL_DYNAMIC_DRAW);
}

void GLBuffer::SetEBOData(const std::vector<unsigned int>& indice_array)
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice_array.size() * sizeof(unsigned int), &indice_array[0], GL_DYNAMIC_DRAW);
}

void GLBuffer::SetEBODataSize(std::size_t n)
{
	ebo_size = n;
}

std::size_t GLBuffer::GetEBODataSize()
{
	return ebo_size;
}

void GLBuffer::AllocateVBOMemo(int location, int tuple_size, unsigned long long buffer_size, unsigned long long offset)
{
	glVertexAttribPointer(location, tuple_size, GL_FLOAT, GL_FALSE, buffer_size, (void*)offset);
	glEnableVertexAttribArray(location);
}

void GLBuffer::FinishInitialization()
{
	glBindVertexArray(0); // VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0); // VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // EBO
}