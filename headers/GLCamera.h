#pragma once
#ifndef GL_CAMERA_H
#define GL_CAMERA_H
#define GLM_ENABLE_EXPERIMENTAL //����ʵ���Թ���֧��
#include <iostream>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp> // ������Ԫ��֧�� 
#include <glm/gtx/quaternion.hpp> // ������Ԫ����չ֧��
enum CAMERA_MOVE_TYPE
{
    FORWARD,
    BACKWARD,
    LEFTWARD,
    RIGHTWARD,
    UPWARD,
    DOWNWARD
};
enum CAMERA_SPIN_TYPE
{
    SPINAROUNDX,
    SPINAROUNDY,
    SPINAROUNDZ
};
class GLCamera
{
    friend void mouse_callback(GLFWwindow* window, double xpos, double ypos);
public:
    GLCamera();
    GLCamera(glm::vec3 camera_pos);
    glm::vec3 GetCameraPos();
    void UpdateSpeedByDeltaTime();
    void ResetCameraPos();
    void SetLookUp(glm::vec3 camera_pos, glm::vec3 camera_target, glm::vec3 vec_up);
    glm::mat4 GetLookUp();
    void SetFOV(float fov);
    float GetFOV() const;
    void SetTargetFOV(float newFOV);
    void UpdateFOV();
    void CameraMove(CAMERA_MOVE_TYPE move_type, float step_move);
    void CameraSpin(CAMERA_SPIN_TYPE spin_type, float spin_angle); // [0,360��] for keyboard
    void CameraSpin(float xoffset, float yoffset); // for mouse
private:
    glm::vec3 cameraPos;  // ���λ��
    glm::vec3 cameraTarget;  // Ŀ��㣨ͨ���ǳ������ģ�
    glm::vec3 up;  // �Ϸ���
    glm::quat orientation; // ʹ����Ԫ����ʾ�������ת
    float cameraSpeed = 0.05f;
    float deltaTime = 0.0f; // ��ǰ֡����һ֡��ʱ���
    float lastFrame = 0.0f; // ��һ֡��ʱ��
    bool firstMouse; // �Ƿ��һ�β������
    float lastX; // ��һ�����λ��X
    float lastY; // ��һ�����λ��Y
    float sensitivity; // ���������
    float currentFOV = 45.0f; // ��ǰ FOV
    float targetFOV = 45.0f;  // Ŀ�� FOV
    float minFOV = 1.0f;      // FOV ��Сֵ
    float maxFOV = 90.0f;     // FOV ���ֵ
};

#endif // !GL_CAMERA_H
