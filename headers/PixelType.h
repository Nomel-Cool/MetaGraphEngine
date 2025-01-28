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

// Todo: 把这些方法和OnePixel中其它方法移动到统一的第三方处理PixelMnager之类的
using StagePos = std::tuple<double, double, double>;

inline bool SamePos(const StagePos& pos_a, const StagePos& pos_b, double abs_epsilon = 1e-12, double rel_epsilon = 1e-8)
{
	double pos_a_x, pos_a_y, pos_a_z, pos_b_x, pos_b_y, pos_b_z;
	std::tie(pos_a_x, pos_a_y, pos_a_z) = pos_a;
	std::tie(pos_b_x, pos_b_y, pos_b_z) = pos_b;
	double diff_x, diff_y, diff_z;
	double bigger_x, bigger_y, bigger_z;
	diff_x = std::fabs(pos_a_x - pos_b_x);
	diff_y = std::fabs(pos_a_y - pos_b_y);
	diff_z = std::fabs(pos_a_z - pos_b_z);
	bigger_x = std::max(std::fabs(pos_a_x), std::fabs(pos_b_x));
	bigger_y = std::max(std::fabs(pos_a_y), std::fabs(pos_b_y));
	bigger_z = std::max(std::fabs(pos_a_z), std::fabs(pos_b_z));
	if (diff_x <= abs_epsilon && diff_y <= abs_epsilon && diff_z <= abs_epsilon)
		return true;
	if (diff_x <= bigger_x * rel_epsilon && diff_y <= bigger_y * rel_epsilon && diff_z <= rel_epsilon)
		return true;
	return false;
}

/// <summary>
/// 像素基类，负责表示最基本的坐标信息，以及与舞台相关的操作
/// </summary>
class OnePixel
{
public:
	OnePixel() = default;
	// 多个子类需要多个重载版本的拷贝构造
	bool render_flag = false, last_flag = false;
	double x = 0.0f, y = 0.0f, z = 0.0f;
	float r = 1.0f, g = 0.5f, b = 0.31f, a = 1.0f, block_size = 1.0f;
	uint64_t cur_frame_id = 0;
	std::size_t tag = 0x0;
	std::map<std::size_t, std::shared_ptr<OnePixel>> owners_info; // <pixel_tag, occupied_pixel_ptr>
	// May be more attributes in a pixel

public:
	std::vector<std::shared_ptr<OnePixel>> GetAllInnerPixels();
	void UpdateSurfaceByMainTag();
	void EmphasizeBeingSingle(bool are_you_insisted);
	bool GetSingleDeclaration();
	void TryUpdatingInnersAccordingToSurface();
	std::shared_ptr<OnePixel> Seperate(std::size_t graph_id);	// 它返回合适筛选条件的视图
	void Merge(std::shared_ptr<OnePixel> sp_merged_pixel);
	void Merge(std::shared_ptr<OnePixel> sp_merged_pixel, bool itself);
	void Merge(std::shared_ptr<OnePixel> sp_merged_pixel, std::size_t specified_tag);

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
