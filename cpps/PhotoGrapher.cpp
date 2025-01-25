#include "PhotoGrapher.h"

std::size_t FrameHashCalculator::GetPixelsHash(const std::vector<OnePixel>& pixels_in_frame) const
{
    std::size_t total_hash = 5381; // 初始化为非零值

    for (const auto& pixel : pixels_in_frame) {
        std::size_t hash = std::hash<std::size_t>()(pixel.x) ^
            (std::hash<std::size_t>()(pixel.y) << 1) ^
            (std::hash<std::size_t>()(pixel.z) >> 1);

        // 改进浮点数哈希方法
        auto hash_float = [](float value) {
            std::size_t float_hash;
            std::memcpy(&float_hash, &value, sizeof(float)); // 将浮点数按位转换为整数
            return std::hash<std::size_t>()(float_hash);
            };

        // 对 r, g, b, a 进行哈希累积
        hash ^= hash_float(pixel.r) ^ hash_float(pixel.g) ^ hash_float(pixel.b) ^ hash_float(pixel.a);

        // 累积到总哈希值
        total_hash ^= hash + 0x9e3779b9 + (total_hash << 6) + (total_hash >> 2); // 更稳定的累积方式
    }

    return total_hash;
}

StaticPhotoGrapher::StaticPhotoGrapher(std::shared_ptr<IFilmStorage> selected_storage_form)
{
    sp_film_storage = selected_storage_form;
}

void StaticPhotoGrapher::Filming(const OnePixel& one_pixel)
{
    sp_film_storage->CollectPixelStream(one_pixel);
}

void StaticPhotoGrapher::FilmDone(const std::string& film_name)
{
    sp_film_storage->Store(film_name);
}

void StaticPhotoGrapher::SetUsageStatus(bool used)
{
    is_using = used;
}

bool StaticPhotoGrapher::GetUsageStatus()
{
    return is_using;
}

RealTimePhotoGrapher::RealTimePhotoGrapher(std::shared_ptr<IFilmStorage> selected_storage_form)
{
    sp_film_storage = selected_storage_form;
}

void RealTimePhotoGrapher::Filming(const OnePixel& one_pixel)
{
    sp_film_storage->CollectPixelStream(one_pixel);
}

void RealTimePhotoGrapher::FilmDone(const std::string& film_name)
{
    sp_film_storage->Store();
}

void RealTimePhotoGrapher::SetUsageStatus(bool used)
{
    is_using = used;
}

bool RealTimePhotoGrapher::GetUsageStatus()
{
    return is_using;
}
