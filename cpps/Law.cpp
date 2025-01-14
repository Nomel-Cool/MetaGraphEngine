#include "Law.h"
#include <cmath>
static float t = 0.0f;
void Gravity::Apply(std::shared_ptr<OnePixel> one_pixel)
{
    //one_pixel->r = std::fmod(one_pixel->r * 255.0 + 10.0f, 255.0f) / 255.0;
    //one_pixel->g = std::fmod(one_pixel->g * 255.0 + 10.0f, 255.0f) / 255.0;
    //one_pixel->b = std::fmod(one_pixel->b * 255.0 + 10.0f, 255.0f) / 255.0;
}

void Gravity::Apply(std::shared_ptr<OnePixel> p_one_pixel, OpInfo op_info)
{
    std::cout << "法则获取界面操作：: " << op_info.op_name << std::endl;
}
