#pragma once
#ifndef GRAPH_GENERATE_ALGORITHM_LIB_H
#define GRAPH_GENERATE_ALGORITHM_LIB_H

#include <memory>

#include "FileManager.h"

#include "IRenderAlgorithm.h"

class BresenhamLineRenderer : public IRenderAlgorithm, public ICoRenderAlgorithm
{
public:
    BresenhamLineRenderer(std::shared_ptr<FileManager> file_manager);
    std::string Execute(const SingleAutomata& graph_model) override;
    ModelGenerator<SingleAutomata> Execute(SingleAutomata& graph_model) override;

private:
    std::shared_ptr<FileManager> sp_file_manager;
};

class JustAPointRenderer : public IRenderAlgorithm, public ICoRenderAlgorithm
{
public:
    JustAPointRenderer(std::shared_ptr<FileManager> file_manager);
    std::string Execute(const SingleAutomata& graph_model) override;
    ModelGenerator<SingleAutomata> Execute(SingleAutomata& graph_model) override;

private:
    std::shared_ptr<FileManager> sp_file_manager;
};

class BresenhamEllipseRenderer : public IRenderAlgorithm, public ICoRenderAlgorithm
{
public:
    BresenhamEllipseRenderer(std::shared_ptr<FileManager> file_manager);
    std::string Execute(const SingleAutomata& graph_model) override;
    ModelGenerator<SingleAutomata> Execute(SingleAutomata& graph_model) override;
private:
    std::shared_ptr<FileManager> sp_file_manager;
};

class PartitionBezierCurveRenderer : public IRenderAlgorithm, public ICoRenderAlgorithm
{
public:
    PartitionBezierCurveRenderer(std::shared_ptr<FileManager> file_manager);
    std::string Execute(const SingleAutomata& graph_model) override;
    ModelGenerator<SingleAutomata> Execute(SingleAutomata& graph_model) override;
private:
    int binomial_coeff(int n, int k);
private:
    std::shared_ptr<FileManager> sp_file_manager;
};

#endif // !GRAPH_GENERATE_ALGORITHM_LIB_H