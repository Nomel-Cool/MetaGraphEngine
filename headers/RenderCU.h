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

// 当像素控件调用生成算法时，默认是查找现有指定算法的 Co 前缀的动态算法，所以像素控件和预览板可以读取同一份xml。
// 预览板使用的是静态一次性生成点集，而像素控件中每个点的生成都是可影响的。
class RenderCU
{
public:
    RenderCU();
    std::function<std::string(const SingleAutomata&)> GetFunctor(std::string func_name);
    std::function<ModelGenerator<SingleAutomata>(SingleAutomata&)> GetCoFunctor(std::string func_name);
protected:
    std::string JustAPoint(const SingleAutomata& graph_model);
    ModelGenerator<SingleAutomata> CoJustAPoint(SingleAutomata& graph_model);

    /// <summary>
    /// 取delta_x, delta_y为起始与终点x与y分量的绝对值
    /// 递推式：pk = 
    /// delta_y-2delta_x, |K| > 1
    /// delta_x-2delta_y, |K| <= 1
    /// 
    /// pk+1=
    /// pk+2delta_x-2delta_y, pk <= 0
    /// pk-2delta_y         , pk > 0
    /// 
    /// </summary>
    /// <param name="graph_model"></param>
    /// <returns></returns>
    std::string BresenhamLine(const SingleAutomata& graph_model);
    ModelGenerator<SingleAutomata> CoBresenhamLine(SingleAutomata& graph_model);

    /// <summary>
    /// 递推式：di = sqrt(2 * r * i - i * i) * a / b，为了效率取di平方再乘b的平方，x往右推进，取八分圆第二部分x<=y为大前提，第二限制x * x < di * di
    /// 每次超过第二限制就让y往下一格
    /// </summary>
    /// <param name="graph_model"></param>
    /// <returns></returns>
    std::string BresenhamEllipse(const SingleAutomata& graph_model);

    std::string PartitionBezierCurve(const SingleAutomata& graph_model);

    // 计算组合数C(n, k)
    int binomial_coeff(int n, int k);
private:
    std::map<std::string, std::function<std::string(const SingleAutomata&)>> render_functions;
    std::map<std::string, std::function<ModelGenerator<SingleAutomata>(SingleAutomata&)>> render_co_functions;
    FileManager file_manager;
};

#endif // !RENDER_CU_H
