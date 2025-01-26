#include "GraphAgency.h"

GraphAgency::GraphAgency(
    std::unique_ptr<IModelLoader> modelLoader,
    std::shared_ptr<IGraphCoHandlerCache> cache,
    std::unique_ptr<ICoroutineManager> coroutineManager)
{
    model_loader = std::move(modelLoader);
    graph_series_cache = std::move(cache);
    coroutine_manager = std::move(coroutineManager);
}

void GraphAgency::LoadGraphs(const QString& model_name)
{
    auto generator = model_loader->LoadModel(model_name);
    graph_series_cache->AddGenerator(std::make_shared<ModelGenerator<SingleAutomata>>(std::move(generator)));
}

void GraphAgency::UpdateGraphs()
{
    coroutine_manager->UpdateAll();
}

const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GraphAgency::GetGraphs()
{
    return graph_series_cache->GetGenerators();
}

bool GraphAgency::Inspect()
{
    return coroutine_manager->CheckAllDone();
}

void GraphAgency::CleanGraphCache()
{
    graph_series_cache->Clear();
}

bool GraphAgency::Empty()
{
    return graph_series_cache->Empty();
}
