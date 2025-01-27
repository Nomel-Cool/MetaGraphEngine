#pragma once
#ifndef PIXEL_TYPE_H
#define PIXEL_TYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <memory>
#include <ranges>
#include <functional>

/// <summary>
/// 像素基类，负责表示最基本的坐标信息，以及与舞台相关的操作
/// </summary>
class OnePixel
{
public:
	OnePixel() = default;
	// 多个子类需要多个重载版本的拷贝构造
	bool render_flag = false, last_flag = false;
	std::size_t x = 0, y = 0, z = 0;
	float r = 1.0f, g = 0.5f, b = 0.31f, a = 1.0f, block_size = 1.0f;
	uint64_t cur_frame_id = 0;
	std::size_t tag = 0x0;
	std::map<std::size_t, std::shared_ptr<OnePixel>> owners_info; // <pixel_tag, occupied_pixel_ptr>
	// May be more attributes in a pixel

public:
	std::vector<std::shared_ptr<OnePixel>> GetAllInnerPixels();
	void TryUpdatingSurfaceIfSinglePixel();
	void TryUpdatingInnerIfSinglePixel();
	void UpdateSurfaceByMainTag();
	void EmphasizeBeingSingle(bool are_you_insisted);
	bool GetSingleDeclaration();
	std::shared_ptr<OnePixel> Seperate(std::size_t graph_id);	// 它返回合适筛选条件的视图
	void Merge(std::shared_ptr<OnePixel> sp_merged_pixel);

private:
	bool insist_being_single = false;
};

class CubePixel : public OnePixel
{
public:
    CubePixel(const OnePixel& basePixel);
	const std::vector<float> GetVertices();
	const std::vector<unsigned int> GetIndices();
	const glm::mat4 GetTransformMat() const;
	const size_t GetVerticesLength() const;
protected:
	void InitializeVertices();
	void InitializeTransformation();
private:
	std::size_t vertex_data_size = 12;
	glm::mat4 R = glm::mat4(1.0f), T = glm::mat4(1.0f), S = glm::mat4(1.0f);
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};


#endif // !PIXEL_TYPE_H
