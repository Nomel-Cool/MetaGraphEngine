#pragma once
#ifndef PIXEL_SPACE_H
#define PIXEL_SPACE_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <algorithm>
#include <vector>
#include <type_traits>
#include <unordered_map>
#include <ranges>

#include "ThreadPool.h"
#include "RenderKernel.h"
#include "Law.h"
#include "PhotoGrapher.h"
#include "PixelType.h"
#include "GLScreen.h"
#include "GraphAgency.h"
#include "Hall.h"
#include "FilmNaming.h"
#include "FrameIDManager.h"
#include "FrameGenerateIntervalManager.h"

using AutomataElements = std::tuple<json, json, json, json>;

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
	std::shared_ptr<FileManager> sp_file_manager;
	std::shared_ptr<FilmNameManager> sp_filmname_manager;
	std::shared_ptr<IFilmStorage> sp_film_static_storage;
	std::shared_ptr<IFilmStorage> sp_film_realtime_storage;
	ShabbyThreadPool& pool = ShabbyThreadPool::GetInstance();
	std::shared_ptr<Law> sp_law; // 由于Law和GraphStudio循环调用了，这里前置声明了Law，使用指针延迟初始化，度过编译期的检查（必须用指针！！！）
	std::shared_ptr<IPhotoGrapher> sp_realtime_photographer, sp_static_photographer;
	std::shared_ptr<QTimer> sp_timer;
	std::shared_ptr<GLScreen> sp_gl_screen;
	std::shared_ptr<FrameIDManager> sp_frame_id_manager;
	std::shared_ptr<FrameGenerateIntervalManager> sp_frame_generate_interval_manager;
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
			std::vector<StagePos> rendered_pixels;

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
			std::vector<StagePos> rendered_pixels;

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