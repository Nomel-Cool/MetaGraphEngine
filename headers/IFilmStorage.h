#pragma once
#ifndef I_FILM_STORAGE_H
#define I_FILM_STORAGE_H

#include <string>

#include "FrameStructure.h"

/// <summary>
/// ��������̬��Ⱦʹ�ã��Ե�Ӱ���д洢
/// </summary>
class IFilmStorage {
public:    
    virtual ~IFilmStorage() = default;
    virtual void CollectPixelStream(const OnePixel& pixel) = 0;
    virtual void Store(const std::string& film_name = "") = 0;
    virtual CompressedFrame Fetch(const std::string& film_name = "") = 0;
    virtual void ClearMemo() = 0;
};

#endif // !I_FILM_STORAGE_H
