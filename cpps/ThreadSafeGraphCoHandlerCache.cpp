#include "ThreadSafeGraphCoHandlerCache.h"

void ThreadSafeGraphCache::AddGenerator(std::shared_ptr<ModelGenerator<SingleAutomata>> generator)
{
    std::lock_guard<std::mutex> lock(mtx);
    generators.emplace_back(std::move(generator));
}

void ThreadSafeGraphCache::Clear()
{
    std::lock_guard<std::mutex> lock(mtx);
    generators.clear();
}

const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& ThreadSafeGraphCache::GetGenerators() const
{
    return generators;
}

bool ThreadSafeGraphCache::Empty() const
{
    return generators.empty();
}

size_t ThreadSafeGraphCache::Size() const
{
    return generators.size();
}
