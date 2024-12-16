#pragma once
#ifndef LAW_H
#define LAW_H

#include <string>
#include <memory>
#include <tuple>
#include "json.hpp"

using json = nlohmann::json;
using AutomataElements = std::tuple<json, json, json, json>;
using PixelElements = std::tuple<float, float, float, float, float, float>;
class Gravity
{
public:
    static PixelElements Apply(AutomataElements& automata_param);
};

#endif // !LAW_H