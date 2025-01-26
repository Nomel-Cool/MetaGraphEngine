#include "RenderAlgorithmFactory.h"

void RenderAlgorithmFactory::RegisterAlgorithm(const std::string& name, std::unique_ptr<IRenderAlgorithm> algo)
{
	renderAlgorithms[name] = std::move(algo);
}

void RenderAlgorithmFactory::RegisterCoAlgorithm(const std::string& name, std::unique_ptr<ICoRenderAlgorithm> algo)
{
	coRenderAlgorithms[name] = std::move(algo);
}

IRenderAlgorithm* RenderAlgorithmFactory::GetAlgorithm(const std::string& name)
{
	auto it = renderAlgorithms.find(name);
	return it != renderAlgorithms.end() ? it->second.get() : nullptr;
}

ICoRenderAlgorithm* RenderAlgorithmFactory::GetCoAlgorithm(const std::string& name)
{
	auto it = coRenderAlgorithms.find(name);
	return it != coRenderAlgorithms.end() ? it->second.get() : nullptr;
}
