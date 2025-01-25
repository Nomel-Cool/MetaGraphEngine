#pragma once
#ifndef I_FILM_STORAGE_H
#define I_FILM_STORAGE_H

#include <string>

#include "FrameStructure.h"

/// <summary>
/// 供给给静态渲染使用，对电影进行存储
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
