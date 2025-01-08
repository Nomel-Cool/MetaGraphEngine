#pragma once
#ifndef GL_CAMERA_H
#define GL_CAMERA_H
#define GLM_ENABLE_EXPERIMENTAL //启用实验性功能支持
#include <iostream>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp> // 包含四元数支持 
#include <glm/gtx/quaternion.hpp> // 包含四元数扩展支持
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
    void CameraSpin(CAMERA_SPIN_TYPE spin_type, float spin_angle); // [0,360°] for keyboard
    void CameraSpin(float xoffset, float yoffset); // for mouse
private:
    glm::vec3 cameraPos;  // 相机位置
    glm::vec3 cameraTarget;  // 目标点（通常是场景中心）
    glm::vec3 up;  // 上方向
    glm::quat orientation; // 使用四元数表示相机的旋转
    float cameraSpeed = 0.05f;
    float deltaTime = 0.0f; // 当前帧与上一帧的时间差
    float lastFrame = 0.0f; // 上一帧的时间
    bool firstMouse; // 是否第一次捕获鼠标
    float lastX; // 上一次鼠标位置X
    float lastY; // 上一次鼠标位置Y
    float sensitivity; // 鼠标灵敏度
    float currentFOV = 45.0f; // 当前 FOV
    float targetFOV = 45.0f;  // 目标 FOV
    float minFOV = 1.0f;      // FOV 最小值
    float maxFOV = 90.0f;     // FOV 最大值
};

#endif // !GL_CAMERA_H
