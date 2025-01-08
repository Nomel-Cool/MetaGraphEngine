#include "GLCamera.h"

GLCamera::GLCamera()
{
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // Ĭ��λ�� 
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Ĭ��Ŀ��� 
	up = glm::vec3(0.0f, 1.0f, 0.0f); // Ĭ���Ϸ���
	orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // ��λ��Ԫ��
	firstMouse = true;
	lastX = 0.0f;
	lastY = 0.0f;
	sensitivity = 0.1f;
}

GLCamera::GLCamera(glm::vec3 camera_pos)
{
	cameraPos = camera_pos; // Ĭ��λ�� 
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Ĭ��Ŀ��� 
	up = glm::vec3(0.0f, 1.0f, 0.0f); // Ĭ���Ϸ���
	orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // ��λ��Ԫ��
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
	cameraSpeed = 2.5f * deltaTime; // ���ò�Ψһ�����豸������������
}

void GLCamera::ResetCameraPos()
{
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // Ĭ��λ�� 
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Ĭ��Ŀ��� 
	up = glm::vec3(0.0f, 1.0f, 0.0f); // Ĭ���Ϸ���
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
	targetFOV = std::clamp(newFOV, minFOV, maxFOV); // ���� FOV �ķ�Χ
}

// ���� FOV������Ⱦѭ���е��ã�
void GLCamera::UpdateFOV()
{
	if (currentFOV != targetFOV) 
	{
		// ʹ�����Բ�ֵƽ������
		float speed = 5.0f; // �����ٶ�
		currentFOV = glm::mix(currentFOV, targetFOV, speed * deltaTime);

		// ����ӽ�Ŀ��ֵ��ֱ������ΪĿ��ֵ
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

void GLCamera::CameraSpin(CAMERA_SPIN_TYPE spin_type, float spin_angle) // [0, 360��]
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
		return; // ��Ч����ֱ�ӷ���
	}

	// ��֤��λ����ת��
	axis = glm::normalize(axis);
	// ����ת�Ƕ�ת��Ϊ����
	float angleRad = glm::radians(spin_angle);
	float halfAngle = angleRad / 2.0f;

	// �ֶ�������Ԫ��
	float sinHalfAngle = sin(halfAngle);
	glm::quat rotation(
		cos(halfAngle),                       // w ����
		axis.x * sinHalfAngle,                // x ����
		axis.y * sinHalfAngle,                // y ����
		axis.z * sinHalfAngle                 // z ����
	);

	// �����������Ԫ������
	orientation = rotation * orientation;

	// ���������λ��
	cameraPos = cameraTarget + orientation * glm::vec3(0.0f, 0.0f, glm::length(cameraPos - cameraTarget));
}

void GLCamera::CameraSpin(float xoffset, float yoffset)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	glm::quat pitch = glm::angleAxis(glm::radians(yoffset), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::quat yaw = glm::angleAxis(glm::radians(-xoffset), glm::vec3(0.0f, 1.0f, 0.0f));

	// ������Ԫ������
	orientation = glm::normalize(yaw * orientation * pitch);

	// �����µ����λ��
	glm::vec3 direction = orientation * glm::vec3(0.0f, 0.0f, -glm::length(cameraPos - cameraTarget));
	cameraPos = cameraTarget - direction;

	// ��ӡ������Ϣ
	// std::cout << "Mouse Offset: " << xoffset << ", " << yoffset << std::endl;
	// std::cout << "CameraPos: " << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << std::endl;
	// std::cout << "CameraTarget: " << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << std::endl;
}


