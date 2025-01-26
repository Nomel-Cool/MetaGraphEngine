#include "RenderCU.h"

RenderCU::RenderCU()
{
    // 如果使用其它第三方XML解析库修改工程指针即可
    std::unique_ptr<shabby::IXMLDocumentFactory> tinyxml_factory = std::make_unique<TinyXMLDocumentFactory>();
    sp_file_manager = std::make_shared<FileManager>(std::move(tinyxml_factory));
}

RenderCU::RenderCU(std::shared_ptr<FileManager> file_manager, std::unique_ptr<RenderAlgorithmFactory> algo_factory)
{
    sp_file_manager = file_manager;
    up_algorithm_factory = std::move(algo_factory);
}

std::string RenderCU::Render(const std::string& algoName, const SingleAutomata& graphModel)
{
    if (auto algo = up_algorithm_factory->GetAlgorithm(algoName))
        return algo->Execute(graphModel);
    return "{}";
}

ModelGenerator<SingleAutomata> RenderCU::CoRender(const std::string& algoName, SingleAutomata& graphModel)
{
    if (auto algo = up_algorithm_factory->GetCoAlgorithm(algoName))
        return algo->Execute(graphModel);
    return ModelGenerator<SingleAutomata>(); // 返回空生成器
}
