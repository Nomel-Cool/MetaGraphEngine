#pragma once
#ifndef GRAPH_AGENCY_H
#define GRAPH_AGENCY_H

#include <memory>

#include "GraphModelLoader.h"
#include "ThreadSafeGraphCoHandlerCache.h"
#include "CoroutineExecutor.h"
/// <summary>
/// 负责协调各个图元进入和退出舞台等候厅
/// </summary>
class GraphAgency
{
public:
	GraphAgency(std::unique_ptr<IModelLoader> modelLoader,
		std::shared_ptr<IGraphCoHandlerCache> cache,
		std::unique_ptr<ICoroutineManager> coroutineManager);
	/// <summary>
	/// QML调用用于把图元生成过程载入像素空间
	/// </summary>
	/// <param name="model_name">载入的模型名</param>
	/// <returns>载入成功为true，否则false</returns>
	void LoadGraphs(const QString& model_name);
	void UpdateGraphs();
	const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGraphs();
	bool Inspect();
	void CleanGraphCache();
	bool Empty();
private:
	std::unique_ptr<IModelLoader> model_loader;
	std::shared_ptr<IGraphCoHandlerCache> graph_series_cache;
	std::unique_ptr<ICoroutineManager> coroutine_manager;
};

#endif // !GRAPH_AGENCY_H
