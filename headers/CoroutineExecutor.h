#pragma once
#ifndef COROUTINE_EXECUTOR_H
#define COROUTINE_EXECUTOR_H

#include <memory>

#include "ICoroutineManager.h"
#include "IGraphCoHandlerCache.h"

class CoroutineExecutor : public ICoroutineManager 
{
public:
    CoroutineExecutor(std::shared_ptr<IGraphCoHandlerCache> cache);

    bool UpdateAll() override;

    bool CheckAllDone() const override;

private:
    std::shared_ptr<IGraphCoHandlerCache> graph_series_cache;
    //std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> graph_series_cache;
};

#endif // !COROUTINE_EXECUTOR_H
