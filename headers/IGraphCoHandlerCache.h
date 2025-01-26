#pragma once
#ifndef I_GRAPH_COHANDLER_CACHE_H
#define I_GRAPH_COHANDLER_CACHE_H

#include <vector>
#include <memory>
#include <mutex>
#include "CoModelGenerator.h"
#include "MetaGraphAutomata.h"

class IGraphCoHandlerCache {
public:
    virtual ~IGraphCoHandlerCache() = default;
    virtual void AddGenerator(std::shared_ptr<ModelGenerator<SingleAutomata>> generator) = 0;
    virtual void Clear() = 0;
    virtual const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GetGenerators() const = 0;
    virtual bool Empty() const = 0;
    virtual size_t Size() const = 0;
};

#endif // !IGraphCoHandlerCache.h
