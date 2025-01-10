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

CubePixel::CubePixel(const OnePixel& basePixel)
{
    // ʹ�ø����������ݳ�ʼ����ǰ����
    x = basePixel.x;
    y = basePixel.y;
    z = basePixel.z;
    r = basePixel.r;
    g = basePixel.g;
    b = basePixel.b;
    a = basePixel.a;
    block_size = basePixel.block_size;
    cur_frame_id = basePixel.cur_frame_id;

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


