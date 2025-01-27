#include "Law.h"
#include <cmath>
static float t = 0.0f;
void Gravity::Apply(std::shared_ptr<OnePixel> p_one_pixel)
{
    //one_pixel->r = std::fmod(one_pixel->r * 255.0 + 10.0f, 255.0f) / 255.0;
    //one_pixel->g = std::fmod(one_pixel->g * 255.0 + 10.0f, 255.0f) / 255.0;
    //one_pixel->b = std::fmod(one_pixel->b * 255.0 + 10.0f, 255.0f) / 255.0;
}

void Gravity::Apply(std::shared_ptr<OnePixel> p_one_pixel, const OpInfo& op_info)
{
    if (p_one_pixel->tag == 0x12)
    {
        p_one_pixel->EmphasizeBeingSingle(true);

    }
    if (op_info.op_frame_id != 0 && op_info.keyboard_info == "SPACE")
    {
        if (p_one_pixel->tag == 0x12)
        {
            p_one_pixel->y += 2;
        }
    }
    if (op_info.op_frame_id != 0 && op_info.keyboard_info == "RIGHT")
    {
        /* auto ptr = p_one_pixel->Seperate(0x11);
         if(ptr != nullptr)
            ptr->y += 1;*/
        auto ptr = p_one_pixel->Seperate(0x11);
            if(ptr != nullptr)
                ptr->x += 1;
    }
    if (p_one_pixel->owners_info.size() == 1 && p_one_pixel->owners_info.find(0x10) != p_one_pixel->owners_info.end())
    {
        p_one_pixel->tag = 0x10;
    }
    //else
    //    p_one_pixel->tag = 17;

    p_one_pixel->y -= p_one_pixel->y > 1 ? 1 : 0;
}
