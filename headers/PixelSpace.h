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
	const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGraphs();
	void CheckModelProcess();
	void CleanGraphCache();
	bool Empty();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
	std::mutex mtx4co;
};

/// <summary>
/// ���ػ��࣬�����ʾ�������������Ϣ���Լ�����̨��صĲ���
/// </summary>
class OnePixel
{
public:
	OnePixel() = default;
	OnePixel(const OnePixel& other) = default;
	OnePixel& operator=(OnePixel& other); // **��Ҫ** �ƽ���������Ȩʱ�ᴥ����ֵ���죬ÿ�����඼���Լ�ʵ��һ����
	// ���������Ҫ������ذ汾�Ŀ�������
	bool render_flag = false;
	bool activate_flag = false;
	std::size_t x = 0, y = 0;
	uint64_t cur_frame_id = 0;
	std::vector<uint64_t> graph_ids;
	// ����и������࣬����������Ӹ��෴ת����ָ��
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
	const uint64_t& GetCurrentFPS() const;
	const std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>>& GetStage() const;
	bool Disable(const std::pair<std::size_t, std::size_t>& coordinate);

	/// <summary>
	/// ֪ͨ�ƽ���������Ȩ�����������ʵ�ֿ�����ֵ����
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