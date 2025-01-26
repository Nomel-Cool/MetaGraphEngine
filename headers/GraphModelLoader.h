#pragma once
#ifndef GRAPH_MODEL_LOADER_H
#define GRAPH_MODEL_LOADER_H

#include "IModelLoader.h"
#include "RenderKernel.h"

class GraphModelLoader : public IModelLoader
{
public:
    GraphModelLoader(std::shared_ptr<GraphFactory> factory);
    ModelGenerator<SingleAutomata> LoadModel(const QString& modelName) override;
private:
    std::shared_ptr<GraphFactory> sp_graph_factory;
};

#endif // !GRAPH_MODEL_LOADER_H
