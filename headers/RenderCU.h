#pragma once
#ifndef RENDER_CU_H
#define RENDER_CU_H

#include <string>
#include <functional>
#include <iostream>
#include <map>
#include <cmath>
#include <algorithm>

#include "FileManager.h"
#include "MetaGraphAutomata.h"
#include "CoModelGenerator.h"
#include "RenderAlgorithmFactory.h"

// 当像素控件调用生成算法时，默认是查找现有指定算法的 Co 前缀的动态算法，所以像素控件和预览板可以读取同一份xml。
// 预览板使用的是静态一次性生成点集，而像素控件中每个点的生成都是可影响的。
class RenderCU
{
public:
    RenderCU();
    RenderCU(std::shared_ptr<FileManager> file_manager, std::unique_ptr<RenderAlgorithmFactory> algo_factory);
    std::string Render(const std::string& algoName, const SingleAutomata& graphModel);
    ModelGenerator<SingleAutomata> CoRender(const std::string& algoName, SingleAutomata& graphModel);
private:
    std::unique_ptr<RenderAlgorithmFactory> up_algorithm_factory;
    std::shared_ptr<FileManager> sp_file_manager;
};

#endif // !RENDER_CU_H
