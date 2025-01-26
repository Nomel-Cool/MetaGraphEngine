#pragma once
#ifndef I_MODEL_LOADER_H
#define I_MODEL_LOADER_H


#include <qstring>

#include "MetaGraphAutomata.h"
#include "CoModelGenerator.h"

class IModelLoader
{
public:
    virtual ~IModelLoader() = default;
    virtual ModelGenerator<SingleAutomata> LoadModel(const QString& modelName) = 0;
};

#endif // !I_MODEL_LOADER_H
