#pragma once
#ifndef GRAPH_GENERATE_ALGORITHM_LIB_H
#define GRAPH_GENERATE_ALGORITHM_LIB_H

#include <memory>
#include <random>

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

class CellAutomataFrame_0 : public IRenderAlgorithm, public ICoRenderAlgorithm
{
public:
    CellAutomataFrame_0(std::shared_ptr<FileManager> file_manager);
    std::string Execute(const SingleAutomata& graph_model) override;
    ModelGenerator<SingleAutomata> Execute(SingleAutomata& graph_model) override;
private:
    std::shared_ptr<FileManager> sp_file_manager;
    bool PetriDish[10][10] = { false };
    void Initialize();
    void Update();
    int CountNeighbors(int x, int y) const;
    void MapPetriDish2Json(json& current_status) const;
    void MapJson2PetriDish(const json& current_status);
};

#endif // !GRAPH_GENERATE_ALGORITHM_LIB_H