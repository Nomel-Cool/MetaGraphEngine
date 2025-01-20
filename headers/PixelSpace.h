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
	const int GetCurrentFrameGenerationInterval() const;
	const std::map<ThreeDCoordinate, std::shared_ptr<OnePixel>>& GetStage() const;
	std::map<ThreeDCoordinate, std::shared_ptr<OnePixel>>::iterator DeleteElementAt(const ThreeDCoordinate& pos);

	/// <summary>
	/// ֪ͨ�ƽ���������Ȩ�����������ʵ�ֿ�����ֵ����
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
	int frame_generation_interval = 18;//ms �������ʵʱ��Ⱦ����֡���ɵ��ٶȣ�����֡������Ⱦ��ֱ��֡��
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
			std::vector<ThreeDCoordinate> rendered_pixels;

			// �����������أ��������һ��
			for (auto iter_pixel = pixels.begin(); iter_pixel != std::prev(pixels.end()); ++iter_pixel)
			{
				if (iter_pixel->second->render_flag != true)
					continue;

				ATTRIBUTE::Apply(iter_pixel->second);
				rendered_pixels.emplace_back(iter_pixel->first);
			}

			// �ػ��������һ������
			if (!pixels.empty())
			{
				auto& last_pixel = *std::prev(pixels.end());
				if (last_pixel.second->render_flag == true)
				{
					last_pixel.second->last_flag = true;  // ���� last_flag Ϊ true
					ATTRIBUTE::Apply(last_pixel.second);
					last_pixel.second->last_flag = false;  // ���� last_flag Ϊ false
					rendered_pixels.emplace_back(last_pixel.first);
				}
			}

			/****************************** ִ���ƽ����� *****************************************/
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

			// �����������أ��������һ��
			for (auto iter_pixel = pixels.begin(); iter_pixel != std::prev(pixels.end()); ++iter_pixel)
			{
				if (iter_pixel->second->render_flag != true)
					continue;

				ATTRIBUTE::Apply(iter_pixel->second, st_op_info);
				rendered_pixels.emplace_back(iter_pixel->first);
			}

			// �ػ��������һ������
			if (!pixels.empty())
			{
				auto& last_pixel = *std::prev(pixels.end());
				if (last_pixel.second->render_flag == true)
				{
					last_pixel.second->last_flag = true;  // ���� last_flag Ϊ true
					ATTRIBUTE::Apply(last_pixel.second, st_op_info);
					last_pixel.second->last_flag = false;  // ���� last_flag Ϊ false
					rendered_pixels.emplace_back(last_pixel.first);
				}
			}

			/****************************** ִ���ƽ����� *****************************************/
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