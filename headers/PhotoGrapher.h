#pragma once
#ifndef PHOTO_GRAPHER_H
#define PHOTO_GRAPHER_H

#include <vector>
#include <map>
#include <string>

#include "PixelType.h"

/// <summary>
/// 压缩一帧的所有渲染信息
/// </summary>
class CompressedFrame
{
public:
	const std::pair<float, float> GetPinPos() const;
	std::vector<OnePixel> GetFrames();
	void UpdateFrames(const std::vector<OnePixel> pixels);
private:
	std::pair<float, float> pin_pos;
	std::vector<OnePixel> frames;
};

class PhotoGrapher
{
public:
	void Store();
	const std::vector<OnePixel> Fetch(const std::string& film_name);
	void Filming(const OnePixel one_pixel);
private:
	std::string current_film_name = "";
	std::map<std::string, CompressedFrame> film_storage;
	std::vector<OnePixel> film_cache;

};

#endif // !PHOTO_GRAPHER_H
