#pragma once
#ifndef GL_MESH_H
#define GL_MESH_H

#include <string>
#include <vector>
#include <memory>

#include "GLShaders.h"
#include "GLBuffers.h"
#include "GLTexture.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class GLMesh {
public:
    /*  ����  */
    GLMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<GLTexture> textures);
    void Draw(GLShader& shader);
private:
    /*  ��������  */
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<GLTexture> textures;
    /*  ��Ⱦ����  */
    std::shared_ptr<GLBuffer> sp_mesh_buffer;
    /*  ����  */
    void SetupMesh();
};

#endif // !GL_MESH_H
