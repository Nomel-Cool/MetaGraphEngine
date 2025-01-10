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
        graph_ids = other.graph_ids; // 所有图元接受同样的法则，所以所有权完全移交没问题

        // 移交后更新源像素
        other.graph_ids.clear();
        other.render_flag = false;
    }
    return *this;
}

CubePixel::CubePixel(const OnePixel& basePixel)
{
    // 使用父类对象的数据初始化当前对象
    x = basePixel.x;
    y = basePixel.y;
    z = basePixel.z;
    r = basePixel.r;
    g = basePixel.g;
    b = basePixel.b;
    a = basePixel.a;
    block_size = basePixel.block_size;
    cur_frame_id = basePixel.cur_frame_id;

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


