#include "Law.h"

PixelElements Gravity::Apply(AutomataElements& automata_param)
{
    float x = 0, y = 0, R = 255.0f, G = 255.0f, B = 255.0f, A = 0.0;

    // �Ҳ�û�����߽��⣬�����Ǹ��յ�json����һ����ȡ��json�ͻ��׳��쳣ʹ�ó��������������ȷ�����ͼԪ��ͼ��ȷʵ���и��ֶ�
    json initial_status, current_status, current_input, terminate_status;
    std::tie(initial_status, current_status, current_input, terminate_status) = automata_param;

    /****************************** �Ը����ص������ߣ�������ǵ���һ��״̬ *****************************************/
    float g = 9.8f, v = 0, delta_t = 1.0f / 120, s = 0;

    // �������ÿ��ģ�͵��Զ������ܹ�����ǰ���򽻻��ģ�����������Щ���ԣ��ٶȡ����ٶȡ������ȵȴ�����
    // ����˴������������Ҵ˴�ֻ��һ��ģ�͵�λ�����Խ��н���
    // λ�õļ�������ɷ��������Ҳ���԰ѷ������Դ��ݸ�ģ�ͼ��������һ�ε���ʱ�������˴�����ǰ��
    for (int i = 0; i < current_status.size(); ++i)
    {
        v = current_input["velocity"][i];
        x = current_status[i]["x"];
        y = current_status[i]["y"];
        s = std::abs(v * delta_t + 0.5f * g * delta_t * delta_t);
        s = v >= 0 ? s : -s;
        if (y >= s)
        {
            // ��û����ײ�����棬���������ٶȺ�λ��
            y -= s;
            v += g * delta_t;
        }
        else
        {
            if (v >= 0)
            {
                // ������棬����ʣ���λ�ƺ��ٶȸ���
                float time_to_ground = std::sqrt(2 * y / g);
                // �ٶȵ������ʱ
                v += g * time_to_ground;
                // ��ײ��ķ����ٶ�˥��
                v = -v * 0.8f;
            }
            else
                v += g * delta_t;
            // λ�õ�����
            //x -= (x == 0 ? 0 : 1);
            y = v < 0 ? y + s : 0;
        }

        // ����current_status�����ã�
        current_status[i]["x"] = x;
        current_status[i]["y"] = y;
        current_input["velocity"][i] = v;
    }

    automata_param = std::make_tuple(initial_status, current_status, current_input, terminate_status);
    return std::make_tuple(x, y, R, G, B, A);
}
