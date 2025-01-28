#include "PixelType.h"

std::vector<std::shared_ptr<OnePixel>> OnePixel::GetAllInnerPixels()
{
    std::vector<std::shared_ptr<OnePixel>> inner_pixels;
    for (const auto& pixel : owners_info)
        inner_pixels.emplace_back(pixel.second);
    return inner_pixels;
}

void OnePixel::UpdateSurfaceByMainTag()
{
    if (owners_info.find(tag) == owners_info.end())
        return;
    r = owners_info[tag]->r;
    g = owners_info[tag]->g;
    b = owners_info[tag]->b;
    a = owners_info[tag]->a;
}

void OnePixel::EmphasizeBeingSingle(bool are_you_insisted)
{
    insist_being_single = are_you_insisted;
}

bool OnePixel::GetSingleDeclaration()
{
    return insist_being_single;
}

void OnePixel::TryUpdatingInnersAccordingToSurface()
{
    for (auto& inner_pixel : owners_info)
    {
        inner_pixel.second->x = x;
        inner_pixel.second->y = y;
        inner_pixel.second->z = z;
    }
}

std::shared_ptr<OnePixel> OnePixel::Seperate(std::size_t tag_info)
{
    if (owners_info.find(tag_info) == owners_info.end() || owners_info.size() == 1) // 禁止左脚踩右脚上天
        return nullptr;
    return owners_info[tag_info];
}

void OnePixel::Merge(std::shared_ptr<OnePixel> sp_merged_pixel)
{
    if (sp_merged_pixel == nullptr)
        return;
    //if (sp_merged_pixel->x != x || sp_merged_pixel->y != y || sp_merged_pixel->z != z)
    if (!SamePos({ sp_merged_pixel->x,sp_merged_pixel->y,sp_merged_pixel->z }, { x,y,z }))
        return;
    for (const auto& [key, value] : sp_merged_pixel->owners_info)
        owners_info[key] = value;
}

void OnePixel::Merge(std::shared_ptr<OnePixel> sp_merged_pixel, bool itself)
{
    if (sp_merged_pixel == nullptr)
        return;
    //if (sp_merged_pixel->x != x || sp_merged_pixel->y != y || sp_merged_pixel->z != z)
    if (!SamePos({ sp_merged_pixel->x,sp_merged_pixel->y,sp_merged_pixel->z }, { x,y,z }))
        return;
    if(itself)
        owners_info[sp_merged_pixel->tag] = sp_merged_pixel;
}

void OnePixel::Merge(std::shared_ptr<OnePixel> sp_merged_pixel, std::size_t specified_tag)
{
    if (sp_merged_pixel == nullptr)
        return;
    //if (sp_merged_pixel->x != x || sp_merged_pixel->y != y || sp_merged_pixel->z != z)
    if (!SamePos({ sp_merged_pixel->x,sp_merged_pixel->y,sp_merged_pixel->z }, { x,y,z }))
        return;
    if (sp_merged_pixel->owners_info.find(specified_tag) == sp_merged_pixel->owners_info.end())
        return;
    owners_info[specified_tag] = sp_merged_pixel->owners_info[specified_tag];
}

CubePixel::CubePixel(const OnePixel& base_pixel)
{
    // 使用父类对象的数据初始化当前对象
    x = base_pixel.x;
    y = base_pixel.y;
    z = base_pixel.z;
    r = base_pixel.r;
    g = base_pixel.g;
    b = base_pixel.b;
    a = base_pixel.a;
    block_size = base_pixel.block_size;
    cur_frame_id = base_pixel.cur_frame_id;

    // 初始化变换
    InitializeTransformation();

    // 初始化顶点数据
    InitializeVertices();
}

const std::vector<float> CubePixel::GetVertices()
{
    return vertices;
}

const std::vector<unsigned int> CubePixel::GetIndices()
{
    return indices;
}

const glm::mat4 CubePixel::GetTransformMat() const
{
    return T * R * S;
}

const size_t CubePixel::GetVerticesLength() const
{
    return vertex_data_size;
}

void CubePixel::InitializeVertices()
{
    // 获取模型矩阵
    glm::mat4 modelMatrix = GetTransformMat();

    // 生成立方体的携带模型矩阵信息的顶点数据
    vertices = {
      // -=======pos======-  -==color==- -=====norm======-  -=sample=-
        // 前面 (法向量指向 -Z)
        -0.5f, -0.5f, -0.5f, r, g, b, a, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // 0 左下
         0.5f, -0.5f, -0.5f, r, g, b, a, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // 1 右下
         0.5f,  0.5f, -0.5f, r, g, b, a, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // 2 右上
        -0.5f,  0.5f, -0.5f, r, g, b, a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // 3 左上

        // 后面 (法向量指向 +Z)
        -0.5f, -0.5f,  0.5f, r, g, b, a, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // 4 右下
         0.5f, -0.5f,  0.5f, r, g, b, a, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 5 左下
         0.5f,  0.5f,  0.5f, r, g, b, a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // 6 左上
        -0.5f,  0.5f,  0.5f, r, g, b, a, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // 7 右上

        // 左面 (法向量指向 -X)
        -0.5f, -0.5f, -0.5f, r, g, b, a, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 8 右下
        -0.5f, -0.5f,  0.5f, r, g, b, a, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 9 左下
        -0.5f,  0.5f,  0.5f, r, g, b, a, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 10 左上
        -0.5f,  0.5f, -0.5f, r, g, b, a, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 11 右上

        // 右面 (法向量指向 +X)
         0.5f, -0.5f, -0.5f, r, g, b, a, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 12 左下
         0.5f, -0.5f,  0.5f, r, g, b, a, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 13 右下
         0.5f,  0.5f,  0.5f, r, g, b, a, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 14 右上
         0.5f,  0.5f, -0.5f, r, g, b, a, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 15 左上

         // 下面 (法向量指向 -Y)
         -0.5f, -0.5f, -0.5f, r, g, b, a, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // 16 左上
          0.5f, -0.5f, -0.5f, r, g, b, a, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // 17 右上
          0.5f, -0.5f,  0.5f, r, g, b, a, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 18 右下
         -0.5f, -0.5f,  0.5f, r, g, b, a, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 19 左下

         // 上面 (法向量指向 +Y)
         -0.5f,  0.5f, -0.5f, r, g, b, a, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // 20 左下
          0.5f,  0.5f, -0.5f, r, g, b, a, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // 21 右下
          0.5f,  0.5f,  0.5f, r, g, b, a, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // 22 右上
         -0.5f,  0.5f,  0.5f, r, g, b, a, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // 23 左上
    };

    // 对每个顶点的位置进行变换
    for (size_t i = 0; i < vertices.size(); i += vertex_data_size)
    {
        // 提取位置向量
        glm::vec4 position(vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);

        // 应用模型矩阵变换
        position = modelMatrix * position;

        // 更新顶点数据中的位置
        vertices[i] = position.x;
        vertices[i + 1] = position.y;
        vertices[i + 2] = position.z;
    }

    // 生成立方体的索引数据
    indices = {
        // 前面
        0, 1, 2,
        2, 3, 0,

        // 后面
        4, 5, 6,
        6, 7, 4,

        // 左面
        8, 9, 10,
        10, 11, 8,

        // 右面
        12, 13, 14,
        14, 15, 12,

        // 下面
        16, 17, 18,
        18, 19, 16,

        // 上面
        20, 21, 22,
        22, 23, 20
    };
}

void CubePixel::InitializeTransformation()
{
    R = glm::mat4(1.0f); // 一般来说不需要旋转
    S = glm::scale(S, glm::vec3(block_size));
    T = glm::translate(T, glm::vec3(x, y, z));
}


