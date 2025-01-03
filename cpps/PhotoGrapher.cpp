#include "PhotoGrapher.h"

const std::pair<float, float> CompressedFrame::GetPinPos() const
{
    return pin_pos;
}

std::vector<OnePixel> CompressedFrame::GetFrames()
{
    return frames;
}

void CompressedFrame::UpdateFrames(const std::vector<OnePixel> pixels)
{
    frames.clear();
    frames = pixels;
}

void PhotoGrapher::Store()
{
    if (current_film_name.empty())
        return;
    CompressedFrame compressed_frame;
    compressed_frame.UpdateFrames(film_cache);
    if (film_storage.find(current_film_name) == film_storage.end())
        film_storage.insert(std::make_pair(current_film_name, std::move(compressed_frame)));
    film_storage[current_film_name].UpdateFrames(film_cache);
    film_cache.clear();
    current_film_name = "";
}

const std::vector<OnePixel> PhotoGrapher::Fetch(const std::string& film_name)
{
    if (film_name.empty() || film_storage.find(film_name) == film_storage.end())
        return {};
    return film_storage[film_name].GetFrames();
}

void PhotoGrapher::Filming(const OnePixel one_pixel)
{
    film_cache.emplace_back(one_pixel);
}
