#pragma once
#ifndef GL_SHADERS_H
#define GL_SHADERS_H

// ���� GLSL ��֪ʶ�ܽ᣺
// 1. ��ͬ����ɫ��֮���໥����
// 2. �������Ƕ�����ָ��in �� out �����๵ͨ
// 3. ֻҪ in �ı���ͬ�� �� ����һ����ɫ���� out ����������Զ������ǵ�ֵ
// 4. uniform �������Դ� CPU �� GPU ���ͱ�����������ȫ�ֱ�����ֱ�����û���£�������������ڣ�
// 5. uniform ���������δ��ʹ�ù�����ᾲĬɾ������Ϊ���ò���Ԥ�⣩

// ��ɫ��Ӳ���룬��������ʱ���루��GLSL, Amazing --- Crazy by a C++ programer.
// GLSL ���������4ά, (x,y,z,w) vecN, ��N������������ά��

#include <glad/glad.h>; // ����glad����ȡ���еı���OpenGLͷ�ļ�
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
    // ����ID
    unsigned int ID;

    // ��������ȡ��������ɫ��
    GLShader(const char* vertexPath, const char* fragmentPath);
    // ʹ��/�������
    void Use();
    // uniform���ߺ���
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
