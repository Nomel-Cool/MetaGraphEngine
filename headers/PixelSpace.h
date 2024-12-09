#pragma once
#ifndef PIXEL_SPACE_H
#define PIXEL_SPACE_H

#include <QObject>
#include <vector>
#include <memory>

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
	void CleanGraphCache();
private:
	std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
};

/// <summary>
/// ������̨���֣�ͼԪ��������̨��Ӱ����̨����
/// </summary>
class Hall
{
public:
	Hall(const std::size_t& stage_width, const std::size_t& stage_height): stage(stage_width, std::vector<OnePixel>(stage_height)) {}
	bool Layout(const std::size_t& row_pixels, const std::size_t& column_pixels);
	void AddRule(std::function<void()> rule); // global lazzy setting
	void AddRule(const std::size_t& i, const std::size_t& j, std::function<void()> rule); // one point
	void AddRule(std::function<void()> set_routine, std::function<void()> rule); // routine
	void CleanSoft(); // turn the rendered pixels into not being. ** Call the QML function in Canvas will be fine.
	void CleanHard();// clear the rendered pixels along with its rules entiredly.
protected:
	void WhenAccessPixel(const std::size_t& i, const std::size_t& j);
private:
	struct OnePixel
	{
		bool render_flag = false;
		std::size_t pixel_width = 1;
		std::size_t pixel_height = 1;
		// rules
		// color
		// ...
		// May be more attributes in a pixel

		void UpdateSize(std::size_t width, std::size_t height);
	};
	std::vector<std::vector<OnePixel>> stage;
};

class GraphStudio : public QObject
{
	Q_OBJECT
public:
	explicit GraphStudio(QObject* parent = nullptr) : QObject(parent) {}
	Q_INVOKABLE void InitHall(const std::size_t& width, const std::size_t& height);
	Q_INVOKABLE void LayoutHall(const std::size_t& row_pixels, const std::size_t& column_pixels);
	Q_INVOKABLE bool RoleEmplacement();
	Q_INVOKABLE void Launch();
	// Record
protected:
	// SnapShot // For recording
private:
	GraphAgency graph_agency;
	std::shared_ptr<Hall> sp_hall;
};

#endif // !PIXEL_SPACE_H