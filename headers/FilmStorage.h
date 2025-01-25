#pragma once
#ifndef FILM_STORAGE_H
#define FILM_STORAGE_H

#include <map>
#include <string>

#include "FrameStructure.h"
#include "IFilmStorage.h"
#include "EventsQueue.h"

class FilmNameMap : public IFilmStorage
{
public:
	void CollectPixelStream(const OnePixel& pixel) override;
	void Store(const std::string& film_name) override;
	CompressedFrame Fetch(const std::string& film_name) override;
	void ClearMemo() override;
private:
	std::vector<OnePixel> film_cache;
	std::map<std::string, CompressedFrame> film_storage;
};

class TaskModelFrameQueue : public IFilmStorage
{
public:
	void CollectPixelStream(const OnePixel& pixel) override;
	void Store(const std::string& film_name) override;
	CompressedFrame Fetch(const std::string& film_name) override;
	void ClearMemo() override;
private:
	std::vector<OnePixel> film_cache;
	TaskModelQueue<CompressedFrame> concurrency_compressedframe_queue;
};

#endif // !FILM_STORAGE_H
