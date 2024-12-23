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

// �����ؿؼ����������㷨ʱ��Ĭ���ǲ�������ָ���㷨�� Co ǰ׺�Ķ�̬�㷨���������ؿؼ���Ԥ������Զ�ȡͬһ��xml��
// Ԥ����ʹ�õ��Ǿ�̬һ�������ɵ㼯�������ؿؼ���ÿ��������ɶ��ǿ�Ӱ��ġ�
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
    /// ȡdelta_x, delta_yΪ��ʼ���յ�x��y�����ľ���ֵ
    /// ����ʽ��pk = 
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
    /// ����ʽ��di = sqrt(2 * r * i - i * i) * a / b��Ϊ��Ч��ȡdiƽ���ٳ�b��ƽ����x�����ƽ���ȡ�˷�Բ�ڶ�����x<=yΪ��ǰ�ᣬ�ڶ�����x * x < di * di
    /// ÿ�γ����ڶ����ƾ���y����һ��
    /// </summary>
    /// <param name="graph_model"></param>
    /// <returns></returns>
    std::string BresenhamEllipse(const SingleAutomata& graph_model);

    std::string PartitionBezierCurve(const SingleAutomata& graph_model);

    // ���������C(n, k)
    int binomial_coeff(int n, int k);
private:
    std::map<std::string, std::function<std::string(const SingleAutomata&)>> render_functions;
    std::map<std::string, std::function<ModelGenerator<SingleAutomata>(SingleAutomata&)>> render_co_functions;
    FileManager file_manager;
};

#endif // !RENDER_CU_H
