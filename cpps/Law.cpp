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
        int distance;  // 距离边界的距离
    };

    std::vector<Position> positions;

    // 统计周围位置的细胞数量，并将空位置加入候选列表
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

    // 如果周围八个位置中已有细胞数量少于3个，进行细胞补充
    if (count < 3) {
        // 按距离远近排序，优先补充离边界远的位置
        std::sort(positions.begin(), positions.end(), [](const Position& a, const Position& b) {
            return a.distance > b.distance;
            });

        // 补充细胞，最多补充至总数达到3个
        for (const auto& pos : positions) {
            if (count >= 3) break;
            int index = (pos.x - x + 1) * 3 + (pos.y - y + 1);
            neighbors[index] = 1;
            count++;
        }
    }
    // 编码到r, g, b
    p_pixel->r = (float)(neighbors[0] + neighbors[1] * 2 + neighbors[2] * 4) / 7;
    p_pixel->g = (float)(neighbors[3] + neighbors[4] * 2 + neighbors[5] * 4) / 7;
    p_pixel->b = (float)(neighbors[6] + neighbors[7] * 2 + neighbors[8] * 4) / 7;
}


void Gravity::Apply(std::shared_ptr<OnePixel> p_one_pixel, const OpInfo& op_info)
{
    if (!p_one_pixel->last_flag)
    {
        // 手动更新表面的a值与里子同步
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
                    // 统计该细胞周围8个位置的细胞数量
                    // 如果是会把自己饿死的情况，就补充最多3个细胞到周围使该细胞避免饿死
                    // 但是需要满足：
                    // 1.添加的细胞尽量离边界远
                    // 2.需要约定左上按顺时针一周的位置编码到cell_matrix[i][j]->r,g,b上，我暂时不知道怎么编码会更易于编程
                    SupplementAndEncode(cell_matrix[i][j], 0, 0);
            }
        }
}
