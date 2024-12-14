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
	const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGraphs();
	void CheckModelProcess();
	void CleanGraphCache();
	bool Empty();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
	std::mutex mtx4co;
};

/// <summary>
/// 像素基类，负责表示最基本的坐标信息，以及与舞台相关的操作
/// </summary>
class OnePixel
{
public:
	OnePixel() = default;
	OnePixel(const OnePixel& other) = default;
	OnePixel& operator=(OnePixel& other); // **重要** 移交像素所有权时会触发赋值构造，每个子类都得自己实现一个。
	// 多个子类需要多个重载版本的拷贝构造
	bool render_flag = false;
	bool activate_flag = false;
	std::size_t x = 0, y = 0;
	uint64_t cur_frame_id = 0;
	std::vector<uint64_t> graph_ids;
	// 如果有更多子类，就在这里添加更多反转控制指针
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
	bool TransferPixelFrom(const std::pair<std::size_t, std::size_t>& coordinate_begin, OnePixel& dest_pixel);
	
	void PingStage(const std::size_t& x, const std::size_t& y, const std::size_t& graph_pos_in_list);

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

class GraphStudio : public QObject
{
	Q_OBJECT
public:
	explicit GraphStudio(QObject* parent = nullptr);
	Q_INVOKABLE void InitHall(const float& width, const float& height);
	Q_INVOKABLE void LayoutHall(const std::size_t& scale_extension);
	Q_INVOKABLE void RoleEmplacement(const QStringList& model_names);
	Q_INVOKABLE QString Display();
	Q_INVOKABLE void Launch();
protected:
	void StandBy();
	void Interact();
	void UpdateGraphList();
	void SnapShot();

private:
	std::tuple<json, json, json, json> GetAutomataInfoAt(std::size_t indice);
	void SetAutomataInfoAt(std::size_t indice, const std::tuple<json, json, json, json>& automata_status);
private:
	std::shared_ptr<GraphAgency> sp_graph_agency;
	std::shared_ptr<Hall> sp_hall;   
	FileManager file_manager;
	ShabbyThreadPool& pool = ShabbyThreadPool::GetInstance();
	CompressedFrame film;
};
#endif // !PIXEL_SPACE_H