#pragma once

#ifndef THREADSAFE_GRAPH_COHANDLER_CACHE_H
#define THREADSAFE_GRAPH_COHANDLER_CACHE_H

#include "IGraphCoHandlerCache.h"

class ThreadSafeGraphCache : public IGraphCoHandlerCache
{
public:
    void AddGenerator(std::shared_ptr<ModelGenerator<SingleAutomata>> generator) override;

    void Clear() override;

    const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGenerators() const override;

    bool Empty() const override;

    size_t Size() const override;

private:
    mutable std::mutex mtx;
    std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>> generators;
};

#endif // !THREADSAFE_GRAPH_COHANDLER_CACHE_H
