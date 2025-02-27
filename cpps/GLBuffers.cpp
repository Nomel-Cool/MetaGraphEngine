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

void* GLBuffer::RetractVBOData(std::size_t vbo_indice = 0)
{
	if (vbo_indice > VBOs.size())
		return nullptr;

	glBindVertexArray(VAO);

	auto vbo = *VBOs[vbo_indice];
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLint buffer_size = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);

	void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, buffer_size, GL_MAP_WRITE_BIT);
	return ptr;
}

/*
 *  以下SetVBOData特化的float类型数据接口暂时失效
*/

//void GLBuffer::SetVBOData(const std::vector<float>& vertex_array, void* ptr, int mod_pos = 0)
//{
//	if (!ptr)
//		return;
//
//	GLint buffer_size = 0;
//	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
//
//	size_t new_data_size = vertex_array.size() * sizeof(float);
//
//	if (mod_pos + new_data_size > buffer_size)
//	{
//		printf("Error: Not enough space in VBO!\n");
//		return;
//	}
//
//	memcpy(ptr, vertex_array.data(), new_data_size);
//
//	glUnmapBuffer(GL_ARRAY_BUFFER);
//}

//void GLBuffer::SetVBOData(std::vector<float> vertex_array)
//{
//	glBindVertexArray(VAO);
//	auto sptr = std::make_shared<unsigned int>();
//	glGenBuffers(1, &(*sptr));
//	glBindBuffer(GL_ARRAY_BUFFER, *sptr);
//	glBufferData(GL_ARRAY_BUFFER, vertex_array.size() * sizeof(float), &vertex_array[0], GL_DYNAMIC_DRAW);
//	VBOs.emplace_back(sptr);
//	buffer_sizes.emplace_back(vertex_array.size());
//}

void GLBuffer::SetEBOData(std::vector<unsigned int> indice_array)
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice_array.size() * sizeof(unsigned int), &indice_array[0], GL_STATIC_DRAW);
}

void GLBuffer::SetEBODataSize(std::size_t n)
{
	ebo_size = n;
}

std::size_t GLBuffer::GetEBODataSize()
{
	return ebo_size;
}

void GLBuffer::AllocateVBOMemo(std::size_t buffer_indice, std::size_t buffer_size)
{
	if (VBOs.empty())
	{
		auto sptr = std::make_shared<unsigned int>();
		glGenBuffers(1, &(*sptr));
		VBOs.emplace_back(sptr);
		buffer_sizes.emplace_back(buffer_size);
	}
	if (buffer_indice > VBOs.size())
		return;
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBOs[buffer_indice]);
	glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
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