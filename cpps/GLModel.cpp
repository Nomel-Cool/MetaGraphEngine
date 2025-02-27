#include "GLModel.h"

GLModel::GLModel(const char* path)
{
    LoadModel(path);
}

void GLModel::Draw(GLShader shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void GLModel::LoadModel(std::string path)
{
    Assimp::Importer import;
    /*
        aiProcess_GenNormals�����ģ�Ͳ������������Ļ�����Ϊÿ�����㴴�����ߡ�
        aiProcess_SplitLargeMeshes�����Ƚϴ������ָ�ɸ�С����������������Ⱦ����󶥵������ƣ�ֻ����Ⱦ��С��������ô����ǳ����á�
        aiProcess_OptimizeMeshes�����ϸ�ѡ���෴�����Ὣ���С����ƴ��Ϊһ��������񣬼��ٻ��Ƶ��ôӶ������Ż���
    */
    const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    ProcessNode(scene->mRootNode, scene);
}

void GLModel::ProcessNode(aiNode* node, const aiScene* scene)
{
    // ����ڵ����е���������еĻ���
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }
    // �������������ӽڵ��ظ���һ����
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

GLMesh GLModel::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<GLTexture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 pos_vec(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        glm::vec3 norm_vec(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        vertex.Position = pos_vec;
        vertex.Normal = norm_vec;
        // Assimp����һ��ģ����һ�������������8����ͬ���������꣬���ǲ����õ���ô�࣬����ֻ���ĵ�һ����������
        if (mesh->mMaterialIndex >= 0)
        {
            glm::vec2 tex_vec(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            vertex.TexCoords = tex_vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }

    // ��������
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // �������
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<GLTexture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<GLTexture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return GLMesh(vertices, indices, textures);
}

std::vector<GLTexture> GLModel::LoadMaterialTextures(aiMaterial* mat, aiTextureType texture_type, std::string type_name)
{
    std::vector<GLTexture> textures;
    // GetTextureCount ��鴢���ڲ�������������������������Ҫһ����������
    for (unsigned int i = 0; i < mat->GetTextureCount(texture_type); i++)
    {
        aiString src_url;
        bool skip = false;
        mat->GetTexture(texture_type, i, &src_url);
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.C_Str(), src_url.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            GLTexture texture(texture_id++); // texture_id����Ŀ == textures�����С - 1�����Ե�����textures�����������Ԫ����Ͱ�ʱ�������������ĳ�����Ϣ��֪��Ӧ�Ĳ��������Ĳ�����Ԫid
            texture.Load2DResource(src_url.C_Str(), true, directory);
            texture.texture_sampler_type = type_name;
            texture.path = src_url;
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}
