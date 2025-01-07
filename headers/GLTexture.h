#pragma once
#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

#include "ImageLoader.h"

enum TEXTURETYPE
{
	_2D,
	_3D
};

class GLTexture
{
public:
	GLTexture() = default;
	GLTexture(unsigned int sampler_id);
	GLTexture(TEXTURETYPE texture_type);
	void SetSamplingMode2D(unsigned int S, unsigned int T);
	void SetSamplingMode2D(glm::vec4 rgba);
	void SetSamplingMode3D(unsigned int S, unsigned int T, unsigned int R);
	void SetSamplingMode3D(glm::vec4 rgba);
	void SetSamplingWhenMinifing2D(unsigned int minify_sample_mode);
	void SetSamplingWhenMinifing3D(unsigned int minify_sample_mode);
	void SetSamplingWhenMagnifing2D(unsigned int magnify_sample_mode);
	void SetSamplingWhenMagnifing3D(unsigned int magnify_sample_mode);
	void ActivateSampler2D() const;
	void ActivateSampler3D() const;
	void Load2DResource(const std::string& url, bool isFlip);
	void Load3DResource(const std::string& url, int source_depth, bool isFlip);
private:
	unsigned int texture = 0, sampler_id = 0;
	int source_width = 0, source_height = 0, source_depth = 0, source_channel = 0;
};

#endif // !GL_TEXTURE_H
