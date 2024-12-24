#pragma once
#ifndef PIXEL_SPACE_H
#define PIXEL_SPACE_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <vector>
#include <memory>
#include <mutex>
#include <any>
#include <type_traits>
#include <tuple>
#include <unordered_map>

#include "ThreadPool.h"
#include "RenderKernel.h"
#include "Law.h"
#include "PixelType.h"
using AutomataElements = std::tuple<json, json, json, json>;
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
	void CleanGraphCache();
	bool Empty();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
	std::mutex mtx4co;
};

/// <summary>
/// 压缩一帧的所有渲染信息
/// </summary>
class CompressedFrame
{
public:
	void Store(const OnePixel& one_pixel);
	const std::vector<OnePixel>& Fetch();
private:
	std::vector<OnePixel> frames;
};

/// <summary>
/// 负责舞台呈现：图元交互，舞台摄影，舞台播放
/// </summary>
class Hall
{
public:
	Hall() = default;
	Hall(const std::size_t& stage_width, const std::size_t& stage_height);
	void Layout(const std::size_t& block_size);
	const std::size_t& GetStageHeight() const;
	const std::size_t& GetStageWidth() const;
	const std::size_t& GetBlockSize() const;
	const uint64_t& GetCurrentFrameID() const;
	const uint64_t& GetCurrentFPS() const;
	const std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>>& GetStage() const;
	bool Disable(const std::pair<std::size_t, std::size_t>& coordinate);

	/// <summary>
	/// 通知移交像素所有权，像素类必须实现拷贝赋值符号
	/// </summary>
	/// <typeparam name="OnePixel"></typeparam>
	/// <param name="coordinate_begin"></param>
	/// <param name="coordinate_end"></param>
	/// <returns></returns>
	bool TransferPixelFrom(const std::pair<std::size_t, std::size_t>& coordinate_begin);
	
	void PingStage(const OnePixel& one_pixel, const std::size_t& graph_pos_in_list);

	//bool SetRule(const std::size_t& x, const std::size_t& y, int pos = -1);
	//void SetRule(std::function<void()> rule); // global lazzy setting
	//void SetRule(std::function<void()> set_routine, std::function<void()> rule); // routine
	//void CleanSoft(); // turn the rendered pixels into not being. ** Call the QML function in Canvas will be fine.
	//void CleanHard();// clear the rendered pixels along with its rules entiredly.
	void NextFrame();

protected:
private:
	std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>> stage;
	std::size_t stage_width = 0, stage_height = 0, block_size = 1;
	std::mutex stage_lock;
	uint64_t frame_id = 1;
	uint64_t FPS = 120;
};

class Law;

class GraphStudio : public QObject
{
	friend class Law;
	Q_OBJECT
public:
	explicit GraphStudio(QObject* parent = nullptr);
	Q_INVOKABLE void InitHall(const float& width, const float& height);
	Q_INVOKABLE void LayoutHall(const std::size_t& scale_extension);
	Q_INVOKABLE void RoleEmplacement(const QStringList& model_names);
	Q_INVOKABLE QString Display();
	Q_INVOKABLE void Launch();
	Q_INVOKABLE void Stop();
protected:
	void StandBy();
	void Interact();
	void UpdateGraphList();
	void SnapShot();

private:
	AutomataElements GetAutomataInfoAt(std::size_t indice);
	void SetAutomataInfoAt(std::size_t indice, const AutomataElements& automata_status);
	// 计算一个数的所有因子
	std::vector<std::size_t> GetFactors(std::size_t n);
	// 计算两个因子序列的交集
	std::vector<std::size_t> GetCommonFactors(const std::vector<std::size_t>& factors1, const std::vector<std::size_t>& factors2);
private:
	std::shared_ptr<GraphAgency> sp_graph_agency;
	std::shared_ptr<Hall> sp_hall;   
	FileManager file_manager;
	ShabbyThreadPool& pool = ShabbyThreadPool::GetInstance();
	std::shared_ptr<Law> sp_law; // 由于Law和GraphStudio循环调用了，这里前置声明了Law，使用指针延迟初始化，度过编译期的检查（必须用指针！！！）
	CompressedFrame film;
	std::shared_ptr<QTimer> sp_timer;
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
			std::vector<std::pair<std::size_t, std::size_t>> rendered_pixels;
			for (const auto& pixel : pixels)
			{
				if (pixel.second->render_flag != true)
					continue;
				ATTRIBUTE::Apply(pixel.second);
				rendered_pixels.emplace_back(pixel.first);
			}
			/****************************** 执行移交手续 *****************************************/
			for (const auto& pos : rendered_pixels)
			{
				bool displace_result = p_studio->sp_hall->TransferPixelFrom(pos);
				if (!displace_result)
					std::cerr << "The displacement is out of range." << std::endl;
			}
		}
		catch (const nlohmann::json::type_error& e)
		{
			std::cerr << "JSON type error while updating JSON fields: " << e.what() << std::endl;
		}
	}

private:
};

#endif // !PIXEL_SPACE_H