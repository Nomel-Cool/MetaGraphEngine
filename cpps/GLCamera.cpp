#include "GLCamera.h"

GLCamera::GLCamera()
{
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // 默认位置 
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // 默认目标点 
	up = glm::vec3(0.0f, 1.0f, 0.0f); // 默认上方向
	orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数
	firstMouse = true;
	lastX = 0.0f;
	lastY = 0.0f;
	sensitivity = 0.1f;
}

GLCamera::GLCamera(glm::vec3 camera_pos)
{
	cameraPos = camera_pos; // 默认位置 
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // 默认目标点 
	up = glm::vec3(0.0f, 1.0f, 0.0f); // 默认上方向
	orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数
	firstMouse = true;
	lastX = 0.0f;
	lastY = 0.0f;
	sensitivity = 0.1f;
}

glm::vec3 GLCamera::GetCameraPos()
{
	return cameraPos;
}

void GLCamera::UpdateSpeedByDeltaTime()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	cameraSpeed = 2.5f * deltaTime; // 设置不唯一，按设备运行流畅调整
}

void GLCamera::ResetCameraPos()
{
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // 默认位置 
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // 默认目标点 
	up = glm::vec3(0.0f, 1.0f, 0.0f); // 默认上方向
}

void GLCamera::SetLookUp(glm::vec3 camera_pos, glm::vec3 camera_target, glm::vec3 vec_up)
{
	cameraPos = camera_pos;
	cameraTarget = camera_target;
	up = vec_up;
}

glm::mat4 GLCamera::GetLookUp()
{
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(cameraPos, cameraTarget, up);
	return view;
}

void GLCamera::SetFOV(float fov)
{
	fov = std::clamp(fov, 1.0f, 90.0f); // Clamp FOV to reasonable limits
}

float GLCamera::GetFOV() const
{
	return currentFOV;
}

void GLCamera::SetTargetFOV(float newFOV)
{
	targetFOV = std::clamp(newFOV, minFOV, maxFOV); // 限制 FOV 的范围
}

// 更新 FOV（在渲染循环中调用）
void GLCamera::UpdateFOV()
{
	if (currentFOV != targetFOV) 
	{
		// 使用线性插值平滑过渡
		float speed = 5.0f; // 调整速度
		currentFOV = glm::mix(currentFOV, targetFOV, speed * deltaTime);

		// 如果接近目标值，直接设置为目标值
		if (std::abs(currentFOV - targetFOV) < 0.01f) {
			currentFOV = targetFOV;
		}
	}
}

void GLCamera::CameraMove(CAMERA_MOVE_TYPE move_type, float step_move)
{
	glm::vec3 forward = glm::normalize(cameraTarget - cameraPos);
	glm::vec3 right = glm::normalize(glm::cross(forward, up)); 
	glm::vec3 move_vec;
	switch (move_type)
	{
	case FORWARD:
		move_vec = forward * step_move * cameraSpeed;
		break;
	case BACKWARD:
		move_vec = -forward * step_move * cameraSpeed;
		break;
	case LEFTWARD:
		move_vec = -right * step_move * cameraSpeed;
		break;
	case RIGHTWARD:
		move_vec = right * step_move * cameraSpeed;
		break;
	case UPWARD:
		move_vec = up * step_move * cameraSpeed;
		break;
	case DOWNWARD:
		move_vec = -up * step_move * cameraSpeed;
		break;
	default:
		break;
	}
	cameraPos += move_vec; 
	cameraTarget += move_vec;
}

void GLCamera::CameraSpin(CAMERA_SPIN_TYPE spin_type, float spin_angle) // [0, 360°]
{
	glm::vec3 axis;
	switch (spin_type)
	{
	case SPINAROUNDX:
		axis = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case SPINAROUNDY:
		axis = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case SPINAROUNDZ:
		axis = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	default:
		return; // 无效类型直接返回
	}

	// 保证单位化旋转轴
	axis = glm::normalize(axis);
	// 将旋转角度转换为弧度
	float angleRad = glm::radians(spin_angle);
	float halfAngle = angleRad / 2.0f;

	// 手动计算四元数
	float sinHalfAngle = sin(halfAngle);
	glm::quat rotation(
		cos(halfAngle),                       // w 分量
		axis.x * sinHalfAngle,                // x 分量
		axis.y * sinHalfAngle,                // y 分量
		axis.z * sinHalfAngle                 // z 分量
	);

	// 更新相机的四元数方向
	orientation = rotation * orientation;

	// 更新相机的位置
	cameraPos = cameraTarget + orientation * glm::vec3(0.0f, 0.0f, glm::length(cameraPos - cameraTarget));
}

void GLCamera::CameraSpin(float xoffset, float yoffset)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	glm::quat pitch = glm::angleAxis(glm::radians(yoffset), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::quat yaw = glm::angleAxis(glm::radians(-xoffset), glm::vec3(0.0f, 1.0f, 0.0f));

	// 更新四元数方向
	orientation = glm::normalize(yaw * orientation * pitch);

	// 计算新的相机位置
	glm::vec3 direction = orientation * glm::vec3(0.0f, 0.0f, -glm::length(cameraPos - cameraTarget));
	cameraPos = cameraTarget - direction;

	// 打印调试信息
	// std::cout << "Mouse Offset: " << xoffset << ", " << yoffset << std::endl;
	// std::cout << "CameraPos: " << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << std::endl;
	// std::cout << "CameraTarget: " << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << std::endl;
}


