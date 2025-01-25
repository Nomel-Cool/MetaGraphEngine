#pragma once
#ifndef PHOTO_GRAPHER_H
#define PHOTO_GRAPHER_H

#include <vector>
#include <map>
#include <string>
#include <functional>
#include <cstddef>
#include <cmath>

#include "PixelType.h"
#include "FrameStructure.h"
#include "FilmStorage.h"
#include "IPhotoGrapher.h"

class FrameHashCalculator
{
public:
	std::size_t GetPixelsHash(const std::vector<OnePixel>& pixels_in_frame) const;
};

class StaticPhotoGrapher : public IPhotoGrapher
{
public:
	StaticPhotoGrapher() = default;
	StaticPhotoGrapher(std::shared_ptr<IFilmStorage> selected_storage_form);
	void Filming(const OnePixel& one_pixel) override;
	void FilmDone(const std::string& film_name = "") override;
	void SetUsageStatus(bool used) override;
	bool GetUsageStatus() override;
private:
	std::shared_ptr<IFilmStorage> sp_film_storage;
	bool is_using = false;
};

class RealTimePhotoGrapher : public IPhotoGrapher
{
public:
	RealTimePhotoGrapher() = default;
	RealTimePhotoGrapher(std::shared_ptr<IFilmStorage> selected_storage_form);
	void Filming(const OnePixel& one_pixel) override;
	void FilmDone(const std::string& film_name = "") override;
	void SetUsageStatus(bool used) override;
	bool GetUsageStatus() override;
private:
	std::shared_ptr<IFilmStorage> sp_film_storage;
	bool is_using = false;
};

#endif // !PHOTO_GRAPHER_H  
