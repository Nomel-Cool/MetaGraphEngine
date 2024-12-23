#pragma once
#ifndef LAW_H
#define LAW_H

#include <string>
#include <memory>
#include <tuple>
#include "json.hpp"

#include "PixelType.h"

using json = nlohmann::json;

class Gravity
{
public:
    static void Apply(std::shared_ptr<OnePixel> p_one_pixel);
};

#endif // !LAW_H