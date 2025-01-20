#pragma once
#ifndef PIXEL_SPACE_H
#define PIXEL_SPACE_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <algorithm>
#include <vector>
#include <memory>
#include <mutex>
#include <any>
#include <type_traits>
#include <tuple>
#include <unordered_map>
#include <ranges>

#include "ThreadPool.h"
#include "RenderKernel.h"
#include "Law.h"
#include "PhotoGrapher.h"
#include "PixelType.h"
#include "GLScreen.h"

using AutomataElements = std::tuple<json, json, json, json>;
using TwoDCoordinate = std::pair<std::size_t, std::size_t>;
using ThreeDCoordinate = std::tuple<std::size_t, std::size_t, std::size_t>;
/// <summary>
/// 负责协调各个图元进入和退出舞台等候厅
/// </summary>
class GraphAgency
{
public:
	/// <summary>
	/// QML调用用于把图元生成过程载入像素空间
	/// </summary>
	/// <param name="model_name">载入的模型名</param>
	/// <returns>载入成功为true，否则false</returns>
	void LoadGraphs(const QString& model_name);
	void UpdateGraphs();
	const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGraphs();
	bool Inspect();
	void CleanGraphCache();
	bool Empty();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
	std::mutex mtx4co;
};

/// <summary>
/// 负责舞台呈现：图元交互，舞台摄影，舞台播放
/// </summary>
class Hall
{
public:
	Hall() = default;
	const uint64_t& GetCurrentFrameID() const;
	const int GetCurrentFrameGenerationInterval() const;
	const std::map<ThreeDCoordinate, std::shared_ptr<OnePixel>>& GetStage() const;
	std::map<ThreeDCoordinate, std::shared_ptr<OnePixel>>::iterator DeleteElementAt(const ThreeDCoordinate& pos);

	/// <summary>
	/// 通知移交像素所有权，像素类必须实现拷贝赋值符号
	/// </summary>
	/// <typeparam name="OnePixel"></typeparam>
	/// <param name="coordinate_begin"></param>
	/// <param name="coordinate_end"></param>
	/// <returns></returns>
	bool TransferPixelFrom(const ThreeDCoordinate& coordinate_begin);
	
	void PingStage(const std::vector<OnePixel>& pixel_list, const std::size_t& graph_pos_in_list);
	std::map<std::size_t, std::vector<OnePixel>> CollectStage();

	void NextFrame();

protected:
private:
	std::map<ThreeDCoordinate, std::shared_ptr<OnePixel>> stage;
	std::map<std::size_t, std::queue<std::shared_ptr<OnePixel>>> checkin_sequence; // <graph_id, {pixel1,...,pixeln}>
	std::mutex stage_lock;
	uint64_t frame_id = 1;
	int frame_generation_interval = 18;//ms 这个数与实时渲染控制帧生成的速度，不是帧动画渲染的直接帧数
};

class Law;

class GraphStudio : public QObject
{
	friend class Law;
	Q_OBJECT
public:
	explicit GraphStudio(QObject* parent = nullptr);
	Q_INVOKABLE void InitWindow(int width, int height, float cameraX, float cameraY, float cameraZ, bool perspective_type, bool view_lock);
	Q_INVOKABLE void SetFilmName(const QString& film_name);
	Q_INVOKABLE void RoleEmplacement(const QStringList& model_names);
	Q_INVOKABLE void Launch();
	Q_INVOKABLE void RealTimeRender();
	Q_INVOKABLE void Ceize();
	Q_INVOKABLE void RoleDismiss();
	Q_INVOKABLE void Display(const QStringList& film_name_list);

signals:
	bool filmTerminated(QString film_name);

protected:
	void StandBy();
	void Interact();
	void RealTimeInteract();
	void UpdateGraphList();
	void SnapShot();
	void RealTimeSnapShot();
	void Stop();
	void TidyUp();

private:
	AutomataElements GetAutomataInfoAt(std::size_t indice);
	void SetAutomataInfoAt(std::size_t indice, const AutomataElements& automata_status);
	void UpdatePixelsThroughAutomata(std::vector<OnePixel>& points, const json& status);
	void UpdateAutomataThroughPixels(json& status, const std::vector<OnePixel>& points);
private:
	std::shared_ptr<GraphAgency> sp_graph_agency;
	std::shared_ptr<Hall> sp_hall;   
	FileManager file_manager;
	ShabbyThreadPool& pool = ShabbyThreadPool::GetInstance();
	std::shared_ptr<Law> sp_law; // 由于Law和GraphStudio循环调用了，这里前置声明了Law，使用指针延迟初始化，度过编译期的检查（必须用指针！！！）
	PhotoGrapher photo_grapher;
	std::shared_ptr<QTimer> sp_timer;
	std::shared_ptr<GLScreen> sp_gl_screen;
};

/// <summary>
/// The Law class has been friended by GraphStudio.
/// </summary>
class Law
{
public:
	template<typename ATTRIBUTE>
	void AffectOn(GraphStudio* p_studio)
	{
		try
		{
			const auto& pixels = p_studio->sp_hall->GetStage();
			std::vector<ThreeDCoordinate> rendered_pixels;

			// 遍历所有像素，除了最后一个
			for (auto iter_pixel = pixels.begin(); iter_pixel != std::prev(pixels.end()); ++iter_pixel)
			{
				if (iter_pixel->second->render_flag != true)
					continue;

				ATTRIBUTE::Apply(iter_pixel->second);
				rendered_pixels.emplace_back(iter_pixel->first);
			}

			// 特化处理最后一个像素
			if (!pixels.empty())
			{
				auto& last_pixel = *std::prev(pixels.end());
				if (last_pixel.second->render_flag == true)
				{
					last_pixel.second->last_flag = true;  // 设置 last_flag 为 true
					ATTRIBUTE::Apply(last_pixel.second);
					last_pixel.second->last_flag = false;  // 重置 last_flag 为 false
					rendered_pixels.emplace_back(last_pixel.first);
				}
			}

			/****************************** 执行移交手续 *****************************************/
			for (const auto& pos : rendered_pixels)
			{
				bool displace_result = p_studio->sp_hall->TransferPixelFrom(pos);
				if (!displace_result)
					std::cerr << "The displacement is out of expectation." << std::endl;
			}
		}
		catch (const nlohmann::json::type_error& e)
		{
			std::cerr << "JSON type error while updating JSON fields: " << e.what() << std::endl;
		}
	}

	template<typename ATTRIBUTE>
	void AffectOn(GraphStudio* p_studio, const OpInfo& st_op_info)
	{
		try
		{
			const auto& pixels = p_studio->sp_hall->GetStage();
			std::vector<ThreeDCoordinate> rendered_pixels;

			// 遍历所有像素，除了最后一个
			for (auto iter_pixel = pixels.begin(); iter_pixel != std::prev(pixels.end()); ++iter_pixel)
			{
				if (iter_pixel->second->render_flag != true)
					continue;

				ATTRIBUTE::Apply(iter_pixel->second, st_op_info);
				rendered_pixels.emplace_back(iter_pixel->first);
			}

			// 特化处理最后一个像素
			if (!pixels.empty())
			{
				auto& last_pixel = *std::prev(pixels.end());
				if (last_pixel.second->render_flag == true)
				{
					last_pixel.second->last_flag = true;  // 设置 last_flag 为 true
					ATTRIBUTE::Apply(last_pixel.second, st_op_info);
					last_pixel.second->last_flag = false;  // 重置 last_flag 为 false
					rendered_pixels.emplace_back(last_pixel.first);
				}
			}

			/****************************** 执行移交手续 *****************************************/
			for (const auto& pos : rendered_pixels)
			{
				bool displace_result = p_studio->sp_hall->TransferPixelFrom(pos);
				if (!displace_result)
					std::cerr << "The displacement is out of expectation." << std::endl;
			}
		}
		catch (const nlohmann::json::type_error& e)
		{
			std::cerr << "JSON type error while updating JSON fields: " << e.what() << std::endl;
		}
	}
};

#endif // !PIXEL_SPACE_H