#pragma once
#ifndef PIXEL_SPACE_H
#define PIXEL_SPACE_H

#include <QObject>
#include <QTimer>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "RenderKernel.h"
#include "ThreadPool.h"

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
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGraphs();
	void CleanGraphCache();
	bool Empty();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
	std::mutex mtx4co;
};
class Hall; // 前向声明

/// <summary>
/// 描述一个像素的状态
/// </summary>
class OnePixel
{
public:
	std::size_t x = 0, y = 0;
	uint64_t pre_frame_id = 0;
	uint64_t cur_frame_id = 0;
	std::vector<std::function<void()>> rules;
	std::vector<uint64_t> graph_ids;
	void Execute(std::shared_ptr<GraphAgency> p_agency, std::shared_ptr<Hall> p_hall);
	// color
	// ...
	// May be more attributes in a pixel
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
	const std::map<std::pair<std::size_t, std::size_t>, OnePixel>& GetStage() const;
	void NextFrame();
	void PingStage(const std::size_t& x, const std::size_t& y, const std::size_t& graph_pos_in_list);
	/// <summary>
	/// 在一点处设置法则，法则的控制范围为整个舞台，以及所有参与的图元，而舞台会记录交互的图元，所以前者蕴含后者，只需要反转控制整个舞台即可。
	/// </summary>
	/// <param name="x">模型的演算点x坐标</param>
	/// <param name="y">模型的演算点y坐标</param>
	/// <returns>若[x,y]位置有像素，则返回true，否则false</returns>
	bool SetRule(const std::size_t& x, const std::size_t& y, int pos = -1);
	//void SetRule(std::function<void()> rule); // global lazzy setting
	//void SetRule(std::function<void()> set_routine, std::function<void()> rule); // routine
	//void CleanSoft(); // turn the rendered pixels into not being. ** Call the QML function in Canvas will be fine.
	//void CleanHard();// clear the rendered pixels along with its rules entiredly.
protected:
private:
	std::map<std::pair<std::size_t, std::size_t>, OnePixel> stage;
	std::size_t stage_width = 0, stage_height = 0, block_size = 1;
	std::mutex stage_lock;
	uint64_t frame_id = 1;
};

class GraphStudio : public QObject
{
	Q_OBJECT
public:
	explicit GraphStudio(QObject* parent = nullptr) : QObject(parent) 
	{
		sp_graph_agency = std::make_shared<GraphAgency>();
		sp_hall = std::make_shared<Hall>();
	}
	Q_INVOKABLE void InitHall(const float& width, const float& height);
	Q_INVOKABLE void LayoutHall(const std::size_t& scale_extension);
	Q_INVOKABLE void RoleEmplacement(const QStringList& model_names);
	Q_INVOKABLE void Launch();
	Q_INVOKABLE QString Display();

protected:
	void StandBy();
	void Render();
	void UpdateGraphList();
	void SnapShot();

protected:
	// SnapShot // For recording
private:
	std::shared_ptr<GraphAgency> sp_graph_agency;
	std::shared_ptr<Hall> sp_hall;   
	FileManager file_manager;
	ShabbyThreadPool& pool = ShabbyThreadPool::GetInstance();
	CompressedFrame film;
};

#endif // !PIXEL_SPACE_H