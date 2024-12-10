#include "PixelSpace.h"

void GraphAgency::LoadGraphs(const QString& model_name)
{
    GraphFactory graph_factory;
    auto graph_model_coroutine_handler = graph_factory.OfferDynamicModel(model_name);
    // �洢Э�̾������̨��
    auto p_co_handler = std::make_shared<ModelGenerator<SingleAutomata>>(graph_model_coroutine_handler);
    graph_series_cache.emplace_back(p_co_handler);
}

void GraphAgency::CleanGraphCache()
{
    if(!graph_series_cache.empty())
        graph_series_cache.clear();
}

bool Hall::Layout(const std::size_t& row_pixels, const std::size_t& column_pixels)
{
    // ����Ƿ��ܹ�����
    if (stage.size() % column_pixels != 0 || stage.begin()->size() % row_pixels != 0)
        return false;

    // ֻ�޸�[0,0]λ�õĴ�С��Ϣ����������ʹ��������
    stage[0][0].pixel_height = stage.size() / column_pixels;
    stage[0][0].pixel_width = stage.begin()->size() / row_pixels;
    return true;
}

void Hall::WhenAccessPixel(const std::size_t& i, const std::size_t& j)
{
    stage[i][j].UpdateSize(stage[0][0].pixel_width, stage[0][0].pixel_height);
    // May be more default action when once access a pixel in [i,j]
}

void Hall::OnePixel::UpdateSize(std::size_t width, std::size_t height)
{
    this->pixel_width = width;
    this->pixel_height = height;
}

void GraphStudio::InitHall(const std::size_t& width, const std::size_t& height)
{
    sp_hall = std::make_shared<Hall>(width, height);
}

void GraphStudio::LayoutHall(const std::size_t& row_pixels, const std::size_t& column_pixels)
{
    sp_hall->Layout(row_pixels, column_pixels);
}
