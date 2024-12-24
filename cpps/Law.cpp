#include "Law.h"
#include <cmath>
static float t = 0.0f;
void Gravity::Apply(std::shared_ptr<OnePixel> one_pixel)
{
    one_pixel->y += 10;
    one_pixel->r = std::fmod(one_pixel->r * 255.0 + 10.0f, 255.0f) / 255.0;
    one_pixel->g = std::fmod(one_pixel->g * 255.0 + 10.0f, 255.0f) / 255.0;
    one_pixel->b = std::fmod(one_pixel->b * 255.0 + 10.0f, 255.0f) / 255.0;
}
