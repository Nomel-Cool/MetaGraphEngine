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
    if (owners_info.find(tag_info) == owners_info.end() || owners_info.size() == 1) // ��ֹ��Ų��ҽ�����
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
    // ʹ�ø����������ݳ�ʼ����ǰ����
    x = base_pixel.x;
    y = base_pixel.y;
    z = base_pixel.z;
    r = base_pixel.r;
    g = base_pixel.g;
    b = base_pixel.b;
    a = base_pixel.a;
    block_size = base_pixel.block_size;
    cur_frame_id = base_pixel.cur_frame_id;

    // ��ʼ���任
    InitializeTransformation();

    // ��ʼ����������
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
    // ��ȡģ�;���
    glm::mat4 modelMatrix = GetTransformMat();

    // �����������Я��ģ�;�����Ϣ�Ķ�������
    vertices = {
      // -=======pos======-  -==color==- -=====norm======-  -=sample=-
        // ǰ�� (������ָ�� -Z)
        -0.5f, -0.5f, -0.5f, r, g, b, a, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // 0 ����
         0.5f, -0.5f, -0.5f, r, g, b, a, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // 1 ����
         0.5f,  0.5f, -0.5f, r, g, b, a, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // 2 ����
        -0.5f,  0.5f, -0.5f, r, g, b, a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // 3 ����

        // ���� (������ָ�� +Z)
        -0.5f, -0.5f,  0.5f, r, g, b, a, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // 4 ����
         0.5f, -0.5f,  0.5f, r, g, b, a, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 5 ����
         0.5f,  0.5f,  0.5f, r, g, b, a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // 6 ����
        -0.5f,  0.5f,  0.5f, r, g, b, a, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // 7 ����

        // ���� (������ָ�� -X)
        -0.5f, -0.5f, -0.5f, r, g, b, a, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 8 ����
        -0.5f, -0.5f,  0.5f, r, g, b, a, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 9 ����
        -0.5f,  0.5f,  0.5f, r, g, b, a, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 10 ����
        -0.5f,  0.5f, -0.5f, r, g, b, a, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 11 ����

        // ���� (������ָ�� +X)
         0.5f, -0.5f, -0.5f, r, g, b, a, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 12 ����
         0.5f, -0.5f,  0.5f, r, g, b, a, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 13 ����
         0.5f,  0.5f,  0.5f, r, g, b, a, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 14 ����
         0.5f,  0.5f, -0.5f, r, g, b, a, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 15 ����

         // ���� (������ָ�� -Y)
         -0.5f, -0.5f, -0.5f, r, g, b, a, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // 16 ����
          0.5f, -0.5f, -0.5f, r, g, b, a, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // 17 ����
          0.5f, -0.5f,  0.5f, r, g, b, a, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 18 ����
         -0.5f, -0.5f,  0.5f, r, g, b, a, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 19 ����

         // ���� (������ָ�� +Y)
         -0.5f,  0.5f, -0.5f, r, g, b, a, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // 20 ����
          0.5f,  0.5f, -0.5f, r, g, b, a, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // 21 ����
          0.5f,  0.5f,  0.5f, r, g, b, a, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // 22 ����
         -0.5f,  0.5f,  0.5f, r, g, b, a, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // 23 ����
    };

    // ��ÿ�������λ�ý��б任
    for (size_t i = 0; i < vertices.size(); i += vertex_data_size)
    {
        // ��ȡλ������
        glm::vec4 position(vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);

        // Ӧ��ģ�;���任
        position = modelMatrix * position;

        // ���¶��������е�λ��
        vertices[i] = position.x;
        vertices[i + 1] = position.y;
        vertices[i + 2] = position.z;
    }

    // �������������������
    indices = {
        // ǰ��
        0, 1, 2,
        2, 3, 0,

        // ����
        4, 5, 6,
        6, 7, 4,

        // ����
        8, 9, 10,
        10, 11, 8,

        // ����
        12, 13, 14,
        14, 15, 12,

        // ����
        16, 17, 18,
        18, 19, 16,

        // ����
        20, 21, 22,
        22, 23, 20
    };
}

void CubePixel::InitializeTransformation()
{
    R = glm::mat4(1.0f); // һ����˵����Ҫ��ת
    S = glm::scale(S, glm::vec3(block_size));
    T = glm::translate(T, glm::vec3(x, y, z));
}


