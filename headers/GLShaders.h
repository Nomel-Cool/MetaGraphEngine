#pragma once
#ifndef GL_SHADERS_H
#define GL_SHADERS_H

// 关于 GLSL 的知识总结：
// 1. 不同的着色器之间相互独立
// 2. 但是它们都可以指定in 和 out 来互相沟通
// 3. 只要 in 的变量同名 与 其它一个着色器的 out 变量，则会自动绑定它们的值
// 4. uniform 变量可以从 CPU 往 GPU 输送变量，但它是全局变量，直至重置或更新（随程序生命周期）
// 5. uniform 变量如果从未被使用过，则会静默删除（行为会变得不可预测）

// 着色器硬编码，会在运行时编译（？GLSL, Amazing --- Crazy by a C++ programer.
// GLSL 中向量最多4维, (x,y,z,w) vecN, 由N给出向量变量维数

#include <glad/glad.h>; // 包含glad来获取所有的必须OpenGL头文件
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class GLShader
{
public:
    // 程序ID
    unsigned int ID;

    // 构造器读取并构建着色器
    GLShader(const char* vertexPath, const char* fragmentPath);
    // 使用/激活程序
    void Use();
    // uniform工具函数
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec3(const std::string& name, glm::vec3 vec) const;
    void SetMat3(const std::string& name, glm::mat3 matrix) const;
    void SetMat4(const std::string& name, glm::mat4 matrix) const;
};

class GLShaders
{
public:

private:

};

#endif // !GL_SHADERS_H
