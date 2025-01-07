#include "GLTexture.h"

GLTexture::GLTexture(unsigned int id)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	sampler_id = id;
}

GLTexture::GLTexture(TEXTURETYPE texture_type)
{
	switch (texture_type)
	{
	case _2D:
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		break;
	case _3D:
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_3D, texture);
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

void GLTexture::Load2DResource(const std::string& url, bool isFlip = false)
{
	stbi_set_flip_vertically_on_load(isFlip); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(url.c_str(), &source_width, &source_height, &source_channel, 0);
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

void GLTexture::Load3DResource(const std::string& url, int source_depth, bool isFlip = false)
{
	stbi_set_flip_vertically_on_load(isFlip); // 告诉 stb_image.h 是否翻转纹理的 y 轴
	unsigned char* data = stbi_load(url.c_str(), &source_width, &source_height, &source_channel, 0);
	if (data)
	{
		GLenum storage_formation;
		if (source_channel == 1)
			storage_formation = GL_RED;
		else if (source_channel == 3)
			storage_formation = GL_RGB;
		else if (source_channel == 4)
			storage_formation = GL_RGBA;

		// 加载 3D 纹理
		glTexImage3D(
			GL_TEXTURE_3D,       // 纹理目标：3D 纹理
			0,                   // Mipmap 级别：0 表示基本级别
			storage_formation,   // 内部格式（例如 GL_RGB, GL_RGBA）
			source_width,        // 纹理宽度
			source_height,       // 纹理高度
			source_depth,        // 纹理深度（3D 纹理的层数）
			0,                   // 边框：必须为 0
			storage_formation,   // 像素数据格式（例如 GL_RGB, GL_RGBA）
			GL_UNSIGNED_BYTE,    // 像素数据类型
			data                 // 像素数据
		);
		glGenerateMipmap(GL_TEXTURE_3D);
	}
	else
	{
		printf("Failed to load texture.\n");
	}
	stbi_image_free(data);
}

