#pragma once
#ifndef PIXEL_TYPE_H
#define PIXEL_TYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <map>
#include <tuple>

/// <summary>
/// ���ػ��࣬�����ʾ�������������Ϣ���Լ�����̨��صĲ���
/// </summary>
class OnePixel
{
public:
	OnePixel() = default;
	OnePixel(const OnePixel& other) = default;
	OnePixel& operator=(OnePixel& other); // **��Ҫ** �ƽ���������Ȩʱ�ᴥ����ֵ���죬ÿ�����඼���Լ�ʵ��һ����
	// ���������Ҫ������ذ汾�Ŀ�������
	bool render_flag = false, last_flag = false;
	std::size_t x = 0, y = 0, z = 0;
	float r = 1.0f, g = 0.5f, b = 0.31f, a = 1.0f, block_size = 1.0f;
	uint64_t cur_frame_id = 0;
	std::map<std::size_t, bool> graph_ids; // <graph_id, activate_flag>
	// May be more attributes in a pixel
};

class CubePixel : public OnePixel
{
public:
    CubePixel(const OnePixel& basePixel);
	const std::vector<float> GetVertices();
	const std::vector<unsigned int> GetIndices();
	const glm::mat4 GetTransformMat() const;
	const size_t GetVerticesLength() const;
protected:
	void InitializeVertices();
	void InitializeTransformation();
private:
	std::size_t vertex_data_size = 12;
	glm::mat4 R = glm::mat4(1.0f), T = glm::mat4(1.0f), S = glm::mat4(1.0f);
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};


#endif // !PIXEL_TYPE_H
