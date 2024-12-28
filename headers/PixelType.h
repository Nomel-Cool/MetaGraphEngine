#pragma once
#ifndef PIXEL_TYPE_H
#define PIXEL_TYPE_H

#include <vector>
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
	bool render_flag = false;
	bool activate_flag = false;
	std::size_t x = 0, y = 0;
	float r = 255.0f, g = 255.0f, b = 255.0f, a = 0.0f, block_size = 1.0f;
	uint64_t cur_frame_id = 0;
	std::vector<uint64_t> graph_ids;
	// May be more attributes in a pixel
};

#endif // !PIXEL_TYPE_H
