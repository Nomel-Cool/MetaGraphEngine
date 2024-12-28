#pragma once
#ifndef PIXEL_TYPE_H
#define PIXEL_TYPE_H

#include <vector>
/// <summary>
/// 像素基类，负责表示最基本的坐标信息，以及与舞台相关的操作
/// </summary>
class OnePixel
{
public:
	OnePixel() = default;
	OnePixel(const OnePixel& other) = default;
	OnePixel& operator=(OnePixel& other); // **重要** 移交像素所有权时会触发赋值构造，每个子类都得自己实现一个。
	// 多个子类需要多个重载版本的拷贝构造
	bool render_flag = false;
	bool activate_flag = false;
	std::size_t x = 0, y = 0;
	float r = 255.0f, g = 255.0f, b = 255.0f, a = 0.0f, block_size = 1.0f;
	uint64_t cur_frame_id = 0;
	std::vector<uint64_t> graph_ids;
	// May be more attributes in a pixel
};

#endif // !PIXEL_TYPE_H
