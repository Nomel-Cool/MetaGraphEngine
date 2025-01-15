#pragma once
#ifndef PHOTO_GRAPHER_H
#define PHOTO_GRAPHER_H

#include <vector>
#include <map>
#include <string>

#include "EventsQueue.h"
#include "PixelType.h"

/// <summary>
/// 压缩一帧的所有渲染信息
/// </summary>
class CompressedFrame
{
public:
	void SetPinPos();
	std::vector<OnePixel> GetFrames() const;
	void UpdateFrames(const std::vector<OnePixel> pixels);
private:
	std::pair<float, float> pin_pos;
	std::vector<OnePixel> frames;
};

class PhotoGrapher
{
public:
	void Store();
	const CompressedFrame Fetch(const std::string& film_name);
	void Filming(const OnePixel one_pixel);
	void RealTimeFilming(const CompressedFrame realtime_frame);
	void RecordFilmName(const std::string& film_name);
	std::string GetCurrentFilmName();
	CompressedFrame TryGettingFrame();
private:
	std::string current_film_name = "";
	std::map<std::string, CompressedFrame> film_storage;
	std::vector<OnePixel> film_cache;
	TaskModelQueue<CompressedFrame> concurrency_compressedframe_queue;
};

#endif // !PHOTO_GRAPHER_H
