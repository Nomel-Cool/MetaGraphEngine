#pragma once
#ifndef GL_SCREEN_H
#define GL_SCREEN_H

#include <map>
#include <vector>
#include <thread>
#include <ranges>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLContext.h"
#include "GLBuffers.h"
#include "GLShaders.h"
#include "GLCamera.h"
#include "GLTexture.h"

#include "PhotoGrapher.h"
#include "PixelType.h"
#include "EventsQueue.h"

// 用于表示渲染一个像素时所用的正多面体类型
enum PixelShape
{
	CUBE = 12,
};

// 用于传递 GUI 中的用户操作信息
struct OpInfo
{
	uint64_t op_frame_id = 0;
	std::string op_name = "Test OP";
};

class GLScreen
{
public:
	GLScreen() = default;
	~GLScreen();
	void InitScreen(int screen_width, int screen_height);
	void InitCamara(glm::vec3 camera_pos);
	void InitPerspectiveType(bool view_type);
	void SetViewLock(bool is_view_lock);
	void SetVerticesData(const std::vector<CubePixel>& cubes);
	std::vector<std::shared_ptr<GLBuffer>> GetFrameBuffers(PixelShape shape_type);
	std::vector<std::shared_ptr<GLBuffer>> GetFrameBuffers(PixelShape shape_type, CompressedFrame a_frame);
	void Rendering();
	void RealTimeRendering(PhotoGrapher& photo_grapher);
	OpInfo TryGettingOpInfo();

protected:

private:
	int width = 0, height = 0;
	bool is_lock = false, perspective_type = false;
	float FPS = 120;
	std::shared_ptr<GLContext> gl_context = std::make_shared<GLContext>();
	std::shared_ptr <GLCamera> gl_camera = std::make_shared<GLCamera>();
	std::thread render_thread, realtime_render_thread;
	std::map<uint64_t, std::vector<CubePixel>> pixel_map;
	TaskModelQueue<OpInfo> concurrency_opinfo_queue;
};

#endif // !GL_SCREEN_H
