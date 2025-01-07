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

#include "ThreadPool.h"
#include "RenderKernel.h"
#include "Law.h"
#include "PhotoGrapher.h"
#include "PixelType.h"
#include "GLScreen.h"

using AutomataElements = std::tuple<json, json, json, json>;
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
	bool Inspect();
	void CleanGraphCache();
	bool Empty();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
	std::mutex mtx4co;
};

/// <summary>
/// ������̨���֣�ͼԪ��������̨��Ӱ����̨����
/// </summary>
class Hall
{
public:
	Hall() = default;
	const uint64_t& GetCurrentFrameID() const;
	const uint64_t& GetCurrentFPS() const;
	const std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>>& GetStage() const;
	std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>>::iterator DeleteElementAt(const std::pair<std::size_t, std::size_t>& pos);
	bool Disable(const std::pair<std::size_t, std::size_t>& coordinate, std::size_t graph_id);

	/// <summary>
	/// ֪ͨ�ƽ���������Ȩ�����������ʵ�ֿ�����ֵ����
	/// </summary>
	/// <typeparam name="OnePixel"></typeparam>
	/// <param name="coordinate_begin"></param>
	/// <param name="coordinate_end"></param>
	/// <returns></returns>
	bool TransferPixelFrom(const std::pair<std::size_t, std::size_t>& coordinate_begin);
	
	void PingStage(const std::vector<OnePixel>& pixel_list, const std::size_t& graph_pos_in_list);
	std::map<std::size_t, std::vector<OnePixel>> CollectStage();

	void NextFrame();

protected:
private:
	std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>> stage;
	std::map<std::size_t, std::queue<std::shared_ptr<OnePixel>>> checkin_sequence; // <graph_id, {pixel1,...,pixeln}>
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
	Q_INVOKABLE void InitWindow(int width, int height);
	Q_INVOKABLE void SetFilmName(const QString& film_name);
	Q_INVOKABLE void RoleEmplacement(const QStringList& model_names);
	Q_INVOKABLE void Launch();
	Q_INVOKABLE void Ceize();
	Q_INVOKABLE void RoleDismiss();
	Q_INVOKABLE void Display(const QStringList& film_name_list);

signals:
	bool filmTerminated(QString film_name);

protected:
	void StandBy();
	void Interact();
	void UpdateGraphList();
	void SnapShot();
	void Stop();
	void TidyUp();

private:
	AutomataElements GetAutomataInfoAt(std::size_t indice);
	void SetAutomataInfoAt(std::size_t indice, const AutomataElements& automata_status);
private:
	std::shared_ptr<GraphAgency> sp_graph_agency;
	std::shared_ptr<Hall> sp_hall;   
	FileManager file_manager;
	ShabbyThreadPool& pool = ShabbyThreadPool::GetInstance();
	std::shared_ptr<Law> sp_law; // ����Law��GraphStudioѭ�������ˣ�����ǰ��������Law��ʹ��ָ���ӳٳ�ʼ�����ȹ������ڵļ�飨������ָ�룡������
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
			std::vector<std::pair<std::size_t, std::size_t>> rendered_pixels;
			for (const auto& pixel : pixels)
			{
				if (pixel.second->render_flag != true)
					continue;
				ATTRIBUTE::Apply(pixel.second);
				rendered_pixels.emplace_back(pixel.first);
			}
			/****************************** ִ���ƽ����� *****************************************/
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