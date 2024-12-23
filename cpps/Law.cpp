#include "Law.h"

PixelElements Gravity::Apply(AutomataElements& automata_param)
{
    float x = 0, y = 0, R = 255.0f, G = 255.0f, B = 255.0f, A = 0.0;

    // 我并没有做边界检测，可能是个空的json，而一旦读取空json就会抛出异常使得程序崩溃，所以请确保你的图元蓝图里确实具有该字段
    json initial_status, current_status, current_input, terminate_status;
    std::tie(initial_status, current_status, current_input, terminate_status) = automata_param;

    /****************************** 对该像素的所有者，算出它们的下一个状态 *****************************************/
    float g = 9.8f, v = 0, delta_t = 1.0f / 120, s = 0;

    // 必须假设每个模型的自动机是能够跟当前法则交互的，即它具有那些属性（速度、加速度、质量等等词条）
    // 假设此处有引力场，且此处只对一个模型的位置属性进行交互
    // 位置的计算可以由法则给出，也可以把法则属性传递给模型计算后在下一次到来时给出，此处采用前者
    for (int i = 0; i < current_status.size(); ++i)
    {
        v = current_input["velocity"][i];
        x = current_status[i]["x"];
        y = current_status[i]["y"];
        s = std::abs(v * delta_t + 0.5f * g * delta_t * delta_t);
        s = v >= 0 ? s : -s;
        if (y >= s)
        {
            // 还没有碰撞到地面，正常更新速度和位移
            y -= s;
            v += g * delta_t;
        }
        else
        {
            if (v >= 0)
            {
                // 到达地面，计算剩余的位移和速度更新
                float time_to_ground = std::sqrt(2 * y / g);
                // 速度到达地面时
                v += g * time_to_ground;
                // 碰撞后的反弹速度衰减
                v = -v * 0.8f;
            }
            else
                v += g * delta_t;
            // 位置到地面
            //x -= (x == 0 ? 0 : 1);
            y = v < 0 ? y + s : 0;
        }

        // 更新current_status（引用）
        current_status[i]["x"] = x;
        current_status[i]["y"] = y;
        current_input["velocity"][i] = v;
    }

    automata_param = std::make_tuple(initial_status, current_status, current_input, terminate_status);
    return std::make_tuple(x, y, R, G, B, A);
}
