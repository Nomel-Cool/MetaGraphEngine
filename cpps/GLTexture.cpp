#include "GLTexture.h"

GLTexture::GLTexture(unsigned int id)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	sampler_id = id;
}

GLTexture::GLTexture(TEXTURETYPE texture_type, unsigned int id)
{
	switch (texture_type)
	{
	case _2D:
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		sampler_id = id;
		break;
	case _3D:
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_3D, texture);
		sampler_id = id;
		break;
	default:
		break;
	}
}

void GLTexture::SetSamplingMode2D(unsigned int sampling_way_in_S, unsigned int sampling_way_in_T)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampling_way_in_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampling_way_in_T);
}

void GLTexture::SetSamplingMode2D(glm::vec4 rgba)
{
	GLfloat border_color[4] = {
		rgba.x,
		rgba.y,
		rgba.z,
		rgba.w
	};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
}

void GLTexture::SetSamplingMode3D(unsigned int sampling_way_in_S, unsigned int sampling_way_in_T, unsigned int sampling_way_in_R)
{
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, sampling_way_in_S);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, sampling_way_in_T);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, sampling_way_in_R);
}

void GLTexture::SetSamplingMode3D(glm::vec4 rgba)
{
	GLfloat border_color[4] = {
	rgba.x,
	rgba.y,
	rgba.z,
	rgba.w
	};
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, border_color);
}

void GLTexture::SetSamplingWhenMinifing2D(unsigned int minify_sample_mode)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minify_sample_mode);
}

void GLTexture::SetSamplingWhenMinifing3D(unsigned int minify_sample_mode)
{
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minify_sample_mode);
}

void GLTexture::SetSamplingWhenMagnifing2D(unsigned int magnify_sample_mode)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnify_sample_mode);
}

void GLTexture::SetSamplingWhenMagnifing3D(unsigned int magnify_sample_mode)
{
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magnify_sample_mode);
}

unsigned int GLTexture::GetSampleID() const
{
	return sampler_id;
}

void GLTexture::ActivateSampler2D() const
{
	glActiveTexture(GL_TEXTURE0 + sampler_id);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void GLTexture::ActivateSampler3D() const
{
	glActiveTexture(GL_TEXTURE0 + sampler_id);
	glBindTexture(GL_TEXTURE_3D, texture);
}

void GLTexture::Load2DResource(const std::string& url, bool isFlip = false, std::string prefix)
{
	std::string _url = url;
	if (!prefix.empty())
		_url = prefix + "/" + url;
	stbi_set_flip_vertically_on_load(isFlip); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(_url.c_str(), &source_width, &source_height, &source_channel, 0);
	if (data)
	{
		GLenum storage_formation;
		if (source_channel == 1)
			storage_formation = GL_RED;
		else if (source_channel == 3)
			storage_formation = GL_RGB;
		else if (source_channel == 4)
			storage_formation = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, storage_formation, source_width, source_height, 0, storage_formation, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		printf("Failed to load texture.\n");
	}
	stbi_image_free(data);
}

void GLTexture::Load3DResource(const std::string& url, int source_depth, bool isFlip = false, std::string prefix)
{
	std::string _url = url;
	if (!prefix.empty())
		_url = prefix + "/" + url;
	stbi_set_flip_vertically_on_load(isFlip); // ���� stb_image.h �Ƿ�ת����� y ��
	unsigned char* data = stbi_load(_url.c_str(), &source_width, &source_height, &source_channel, 0);
	if (data)
	{
		GLenum storage_formation;
		if (source_channel == 1)
			storage_formation = GL_RED;
		else if (source_channel == 3)
			storage_formation = GL_RGB;
		else if (source_channel == 4)
			storage_formation = GL_RGBA;

		// ���� 3D ����
		glTexImage3D(
			GL_TEXTURE_3D,       // ����Ŀ�꣺3D ����
			0,                   // Mipmap ����0 ��ʾ��������
			storage_formation,   // �ڲ���ʽ������ GL_RGB, GL_RGBA��
			source_width,        // ������
			source_height,       // ����߶�
			source_depth,        // ������ȣ�3D ����Ĳ�����
			0,                   // �߿򣺱���Ϊ 0
			storage_formation,   // �������ݸ�ʽ������ GL_RGB, GL_RGBA��
			GL_UNSIGNED_BYTE,    // ������������
			data                 // ��������
		);
		glGenerateMipmap(GL_TEXTURE_3D);
	}
	else
	{
		printf("Failed to load texture.\n");
	}
	stbi_image_free(data);
}

