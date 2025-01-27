#include "GraphModelLoader.h"

GraphModelLoader::GraphModelLoader(std::shared_ptr<GraphFactory> factory)
{
	sp_graph_factory = factory;
}

ModelGenerator<SingleAutomata> GraphModelLoader::LoadModel(const QString& model_name)
{
	std::lock_guard<std::mutex> lock(mtx);
    return sp_graph_factory->OfferDynamicModel(model_name);
}
