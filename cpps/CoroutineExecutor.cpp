#include "CoroutineExecutor.h"

CoroutineExecutor::CoroutineExecutor(std::shared_ptr<IGraphCoHandlerCache> cache)
{
    graph_series_cache = cache;
}

bool CoroutineExecutor::UpdateAll()
{
    bool allDone = true;
    for (auto& generator : graph_series_cache->GetGenerators())
        if (!generator->Done())
        {
            generator->Resume();
            allDone = false;
        }
    return !allDone;
}

bool CoroutineExecutor::CheckAllDone() const
{
    for (const auto& generator : graph_series_cache->GetGenerators())
        if (!generator->Done()) 
            return false;
    return true;
}
