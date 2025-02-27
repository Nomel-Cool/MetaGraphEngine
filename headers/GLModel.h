#pragma once
#ifndef GL_MODEL_H
#define GL_MODEL_H

#include <string>
#include <vector>

#include "GLMesh.h"
#include "GLShaders.h"

class GLModel
{
public:
    GLModel(const char* path);
    void Draw(GLShader shader);
private:
    /* �Ż��ã��ظ�����Ĳ������ɶ���ֱ��ʹ�� */
    std::vector<GLTexture> textures_loaded;
    /*  ģ������  */
    std::vector<GLMesh> meshes;
    std::string directory;
    unsigned int texture_id = 0;
    /*  ����   */
    void LoadModel(std::string path);
    void ProcessNode(aiNode* node, const aiScene* scene);
    GLMesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<GLTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

#endif // !GL_MODEL_H
