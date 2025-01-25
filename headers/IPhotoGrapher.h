#pragma once
#ifndef I_PHOTOGRAPHER_H
#define I_PHOTOGRAPHER_H

#include "PixelType.h"

class IPhotoGrapher
{
public:
	virtual void Filming(const OnePixel& one_pixel) = 0;
	virtual void FilmDone(const std::string& film_name = "") = 0;
	virtual bool GetUsageStatus() = 0;
	virtual void SetUsageStatus(bool used) = 0;
};

#endif // !I_PHOTOGRAPHER_H
