#pragma once
#ifndef FRAME_STRUCTURE_H
#define FRAME_STRUCTURE_H

#include <vector>

#include "PixelType.h"

/// <summary>
/// ѹ��һ֡��������Ⱦ��Ϣ
/// </summary>
class CompressedFrame
{
public:
	std::vector<OnePixel> GetFrames() const;
	void UpdateFrames(const std::vector<OnePixel>& pixels);
private:
	std::vector<OnePixel> frames;
};

#endif // !FRAME_STRUCTURE_H
