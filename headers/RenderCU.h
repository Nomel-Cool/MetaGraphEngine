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

// �����ؿؼ����������㷨ʱ��Ĭ���ǲ�������ָ���㷨�� Co ǰ׺�Ķ�̬�㷨���������ؿؼ���Ԥ������Զ�ȡͬһ��xml��
// Ԥ����ʹ�õ��Ǿ�̬һ�������ɵ㼯�������ؿؼ���ÿ��������ɶ��ǿ�Ӱ��ġ�
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
