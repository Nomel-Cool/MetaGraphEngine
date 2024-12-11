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
/// ����Э������ͼԪ������˳���̨�Ⱥ���
/// </summary>
class GraphAgency
{
public:
	/// <summary>
	/// QML�������ڰ�ͼԪ���ɹ����������ؿռ�
	/// </summary>
	/// <param name="model_name">�����ģ����</param>
	/// <returns>����ɹ�Ϊtrue������false</returns>
	void LoadGraphs(const QString& model_name);
	void UpdateGraphs();
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGraphs();
	void CleanGraphCache();
	bool Empty();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
	std::mutex mtx4co;
};
class Hall; // ǰ������

/// <summary>
/// ����һ�����ص�״̬
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
/// ѹ��һ֡��������Ⱦ��Ϣ
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
/// ������̨���֣�ͼԪ��������̨��Ӱ����̨����
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
	/// ��һ�㴦���÷��򣬷���Ŀ��Ʒ�ΧΪ������̨���Լ����в����ͼԪ������̨���¼������ͼԪ������ǰ���̺����ߣ�ֻ��Ҫ��ת����������̨���ɡ�
	/// </summary>
	/// <param name="x">ģ�͵������x����</param>
	/// <param name="y">ģ�͵������y����</param>
	/// <returns>��[x,y]λ�������أ��򷵻�true������false</returns>
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