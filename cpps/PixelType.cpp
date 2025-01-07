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
        block_size = other.block_size;
        render_flag = other.render_flag;
        cur_frame_id = other.cur_frame_id;
        graph_ids = other.graph_ids; // ����ͼԪ����ͬ���ķ�����������Ȩ��ȫ�ƽ�û����

        // �ƽ������Դ����
        other.graph_ids.clear();
        other.render_flag = false;
    }
    return *this;
}