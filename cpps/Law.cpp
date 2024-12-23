#include "Law.h"

static float t = 0.0f;
void Gravity::Apply(std::shared_ptr<OnePixel> one_pixel)
{
    float x = 0, y = 0, R = 255.0f, G = 255.0f, B = 255.0f, A = 0.0;
    // 简单地将像素y轴上移动 10 个像素
    one_pixel->y += 10;
}
