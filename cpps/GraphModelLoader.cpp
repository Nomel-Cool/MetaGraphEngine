#include "GraphModelLoader.h"

GraphModelLoader::GraphModelLoader(std::shared_ptr<GraphFactory> factory)
{
	sp_graph_factory = factory;
}

ModelGenerator<SingleAutomata> GraphModelLoader::LoadModel(const QString& modelName)
{
    return sp_graph_factory->OfferDynamicModel(modelName);
}
