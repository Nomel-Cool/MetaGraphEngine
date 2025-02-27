#include "GLMesh.h"

GLMesh::GLMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<GLTexture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    sp_mesh_buffer = std::make_shared<GLBuffer>();
    SetupMesh();
}

void GLMesh::Draw(GLShader& shader)
{
    unsigned int diffuseNr = 1, specularNr = 1, normalNr = 1, heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        // 切换纹理，同一个物体上有高光贴图和原贴图
        textures[i].ActivateSampler2D();

        std::string number = "";
        std::string name = textures[i].texture_sampler_type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);
        shader.Use();
        shader.SetInt(("material." + name + number).c_str(), textures[i].GetSampleID()); // 为采样器挂一个调用id
    }

    // 绘制网格
    sp_mesh_buffer->EnableVAO();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    sp_mesh_buffer->DisableVAO();

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void GLMesh::SetupMesh()
{
    sp_mesh_buffer->SetVBOData(vertices);
    sp_mesh_buffer->AllocateVBOMemo(0, 3, sizeof(Vertex), 0);                           // 位置
    //sp_mesh_buffer->AllocateVBOMemo(1, 4, sizeof(Vertex), offsetof(Vertex,Color));    // 颜色
    sp_mesh_buffer->AllocateVBOMemo(2, 3, sizeof(Vertex), offsetof(Vertex, Normal));    // 法线
    sp_mesh_buffer->AllocateVBOMemo(3, 2, sizeof(Vertex), offsetof(Vertex, TexCoords)); // 纹理坐标
    sp_mesh_buffer->SetEBOData(indices);
    sp_mesh_buffer->SetEBODataSize(indices.size());
    sp_mesh_buffer->FinishInitialization();
}
