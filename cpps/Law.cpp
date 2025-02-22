#include "Law.h"
#include <cmath>
static std::shared_ptr<OnePixel> cell_matrix[30][30] = { nullptr };
void Gravity::Apply(std::shared_ptr<OnePixel> p_one_pixel)
{
    //one_pixel->r = std::fmod(one_pixel->r * 255.0 + 10.0f, 255.0f) / 255.0;
    //one_pixel->g = std::fmod(one_pixel->g * 255.0 + 10.0f, 255.0f) / 255.0;
    //one_pixel->b = std::fmod(one_pixel->b * 255.0 + 10.0f, 255.0f) / 255.0;
}

void SupplementAndEncode(std::shared_ptr<OnePixel> p_pixel, int dx, int dy) {
    int x = (int)p_pixel->x;
    int y = (int)p_pixel->y;
    int count = 0;
    int neighbors[9] = { 0,0,0,0,1,0,0,0,0 };

    struct Position {
        int x, y;
        int distance;  // ����߽�ľ���
    };

    std::vector<Position> positions;

    // ͳ����Χλ�õ�ϸ��������������λ�ü����ѡ�б�
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;
            int nx = x + i;
            int ny = y + j;
            if (nx >= 0 + dx && nx < 10 + dy && ny >= 0 + dx && ny < 10 + dy) {
                int index = (i + 1) * 3 + (j + 1);
                if (cell_matrix[nx][ny]->a != 0.0f) {
                    neighbors[index] = 1;
                    count++;
                }
                else {
                    int distance = 0;
                    if (nx >= 0)
                        distance = std::min({ nx, 10 + dx - nx });
                    else if (ny >= 0)
                        distance = std::min({ ny, 10 + dy - ny });
                    else
                        distance = 30;
                    positions.push_back({ nx, ny, distance });
                }
            }
        }
    }

    // �����Χ�˸�λ��������ϸ����������3��������ϸ������
    if (count < 3) {
        // ������Զ���������Ȳ�����߽�Զ��λ��
        std::sort(positions.begin(), positions.end(), [](const Position& a, const Position& b) {
            return a.distance > b.distance;
            });

        // ����ϸ������ಹ���������ﵽ3��
        for (const auto& pos : positions) {
            if (count >= 3) break;
            int index = (pos.x - x + 1) * 3 + (pos.y - y + 1);
            neighbors[index] = 1;
            count++;
        }
    }
    // ���뵽r, g, b
    p_pixel->r = (float)(neighbors[0] + neighbors[1] * 2 + neighbors[2] * 4) / 7;
    p_pixel->g = (float)(neighbors[3] + neighbors[4] * 2 + neighbors[5] * 4) / 7;
    p_pixel->b = (float)(neighbors[6] + neighbors[7] * 2 + neighbors[8] * 4) / 7;
}


void Gravity::Apply(std::shared_ptr<OnePixel> p_one_pixel, const OpInfo& op_info)
{
    if (!p_one_pixel->last_flag)
    {
        // �ֶ����±����aֵ������ͬ��
        assert(p_one_pixel->owners_info.size() == 1);
        p_one_pixel->a = p_one_pixel->owners_info.begin()->second->a;
        p_one_pixel->r = p_one_pixel->owners_info.begin()->second->r;
        p_one_pixel->g = p_one_pixel->owners_info.begin()->second->g;
        p_one_pixel->b = p_one_pixel->owners_info.begin()->second->b;
        cell_matrix[(int)p_one_pixel->x][(int)p_one_pixel->y] = p_one_pixel;
        return;
    }
    assert(p_one_pixel->owners_info.size() == 1);
    p_one_pixel->a = p_one_pixel->owners_info.begin()->second->a;
    p_one_pixel->r = p_one_pixel->owners_info.begin()->second->r;
    p_one_pixel->g = p_one_pixel->owners_info.begin()->second->g;
    p_one_pixel->b = p_one_pixel->owners_info.begin()->second->b;
    cell_matrix[(int)p_one_pixel->x][(int)p_one_pixel->y] = p_one_pixel;

    for(int i = 0; i < 30; ++i)
        for (int j = 0; j < 30; ++j)
        {
            if(cell_matrix[i][j] != nullptr)
            {
                if(cell_matrix[i][j]->a != 0.0f)
                    // ͳ�Ƹ�ϸ����Χ8��λ�õ�ϸ������
                    // ����ǻ���Լ�������������Ͳ������3��ϸ������Χʹ��ϸ���������
                    // ������Ҫ���㣺
                    // 1.��ӵ�ϸ��������߽�Զ
                    // 2.��ҪԼ�����ϰ�˳ʱ��һ�ܵ�λ�ñ��뵽cell_matrix[i][j]->r,g,b�ϣ�����ʱ��֪����ô���������ڱ��
                    SupplementAndEncode(cell_matrix[i][j], 0, 0);
            }
        }
}
