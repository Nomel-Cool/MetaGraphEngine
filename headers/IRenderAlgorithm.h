#pragma once
#ifndef I_RENDER_ALGORITHM_H
#define I_RENDER_ALGORITHM_H

#include <string>

#include "MetaGraphAutomata.h"
#include "CoModelGenerator.h"

class IRenderAlgorithm 
{
public:
    virtual ~IRenderAlgorithm() = default;
    virtual std::string Execute(const SingleAutomata& graphModel) = 0;
};

class ICoRenderAlgorithm
{
public:
    virtual ~ICoRenderAlgorithm() = default;
    virtual ModelGenerator<SingleAutomata> Execute(SingleAutomata& graphModel) = 0;
};

#endif // !I_RENDER_ALGORITHM_H
