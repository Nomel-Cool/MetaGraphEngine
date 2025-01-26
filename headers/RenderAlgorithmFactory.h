#pragma once
#ifndef RENDER_ALGORITHM_FACTORY_H
#define RENDER_ALGORITHM_FACTORY_H

#include <map>
#include <string>
#include <memory>

#include "IRenderAlgorithm.h"

class RenderAlgorithmFactory 
{
public:
    using RenderMap = std::map<std::string, std::unique_ptr<IRenderAlgorithm>>;
    using CoRenderMap = std::map<std::string, std::unique_ptr<ICoRenderAlgorithm>>;

    RenderAlgorithmFactory() = default;

    void RegisterAlgorithm(const std::string& name, std::unique_ptr<IRenderAlgorithm> algo);
    void RegisterCoAlgorithm(const std::string& name, std::unique_ptr<ICoRenderAlgorithm> algo);

    IRenderAlgorithm* GetAlgorithm(const std::string& name);
    ICoRenderAlgorithm* GetCoAlgorithm(const std::string& name);

private:
    RenderMap renderAlgorithms;
    CoRenderMap coRenderAlgorithms;
};

#endif // !RENDER_ALGORITHM_FACTORY_H
