#include "PixelType.h"

OnePixel& OnePixel::operator=(OnePixel& other)
{
    if (this != &other)
    {
        x = other.x;
        y = other.y;
        r = other.r;
        g = other.g;
        b = other.b;
        a = other.a;
        activate_flag = other.activate_flag;
        render_flag = other.render_flag;
        cur_frame_id = other.cur_frame_id;
        graph_ids = other.graph_ids; // 所有图元接受同样的法则，所以所有权完全移交没问题

        // 移交后更新源像素
        other.graph_ids.clear();
        other.render_flag = false;
        other.activate_flag = false;
    }
    return *this;
}