#pragma once
#ifndef PIXEL_SPACE_H
#define PIXEL_SPACE_H

#include <QObject>
#include <QTimer>
#include <vector>
#include <memory>

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
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGraphs();
	void CleanGraphCache();
	bool Empty();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
};
class Hall; // 前向声明
class OnePixel
{
public:
	bool render_flag = false;
	std::size_t block_size = 1;
	std::size_t x;
	std::size_t y;
	// rules
	// color
	// ...
	// May be more attributes in a pixel

	void UpdatePos(const std::size_t& x, const std::size_t& y);
	void UpdateSize(const std::size_t& block_size);
	OnePixel ApplyRule(Hall& hall);
};

/// <summary>
/// 负责舞台呈现：图元交互，舞台摄影，舞台播放
/// </summary>
class Hall
{
public:
	Hall() = default;
	Hall(const std::size_t& stage_width, const std::size_t& stage_height): stage(stage_width, std::vector<OnePixel>(stage_height)) {}
	void Layout(const std::size_t& block_size);
	std::size_t GetStageHeight();
	std::size_t GetStageWidth();
	OnePixel SetStage(std::size_t i, std::size_t j);
	std::vector<std::vector<OnePixel>>& GetStage();
	//void AddRule(std::function<void()> rule); // global lazzy setting
	//void AddRule(const std::size_t& i, const std::size_t& j, std::function<void()> rule); // one point
	//void AddRule(std::function<void()> set_routine, std::function<void()> rule); // routine
	//void CleanSoft(); // turn the rendered pixels into not being. ** Call the QML function in Canvas will be fine.
	//void CleanHard();// clear the rendered pixels along with its rules entiredly.
protected:
	void WhenAccessPixel(const std::size_t& i, const std::size_t& j);
private:
	std::vector<std::vector<OnePixel>> stage;
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
	Q_INVOKABLE bool RoleEmplacement(const QString& model_name);
	Q_INVOKABLE void Launch();
	// Record

signals:
	void drawPixeled(float x, float y, float blockSize);

protected:
	// SnapShot // For recording
private:
	std::shared_ptr<GraphAgency> sp_graph_agency;
	std::shared_ptr<Hall> sp_hall;   
	FileManager file_manager;
};

#endif // !PIXEL_SPACE_H