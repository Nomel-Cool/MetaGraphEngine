#include "PhotoGrapher.h"

void CompressedFrame::SetPinPos()
{
    for (OnePixel& pic : frames)
    {
        pic.x += pin_pos.first;
        pic.y += pin_pos.second;
    }
}

std::vector<OnePixel> CompressedFrame::GetFrames() const
{
    return frames;
}

void CompressedFrame::UpdateFrames(const std::vector<OnePixel>& pixels)
{
    frames.clear();
    frames = pixels;
}

std::size_t CompressedFrame::GetPixelsHash() const 
{
    std::size_t total_hash = 5381; // ��ʼ��Ϊ����ֵ

    for (const auto& pixel : frames) {
        std::size_t hash = std::hash<std::size_t>()(pixel.x) ^
            (std::hash<std::size_t>()(pixel.y) << 1) ^
            (std::hash<std::size_t>()(pixel.z) >> 1);

        // �Ľ���������ϣ����
        auto hash_float = [](float value) {
            std::size_t float_hash;
            std::memcpy(&float_hash, &value, sizeof(float)); // ����������λת��Ϊ����
            return std::hash<std::size_t>()(float_hash);
            };

        // �� r, g, b, a ���й�ϣ�ۻ�
        hash ^= hash_float(pixel.r) ^ hash_float(pixel.g) ^ hash_float(pixel.b) ^ hash_float(pixel.a);

        // �ۻ����ܹ�ϣֵ
        total_hash ^= hash + 0x9e3779b9 + (total_hash << 6) + (total_hash >> 2); // ���ȶ����ۻ���ʽ
    }

    return total_hash;
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

const CompressedFrame PhotoGrapher::Fetch(const std::string& film_name)
{
    if (film_name.empty() || film_storage.find(film_name) == film_storage.end())
        return {};
    return film_storage[film_name];
}

void PhotoGrapher::Filming(const OnePixel one_pixel)
{
    film_cache.emplace_back(one_pixel);
}

void PhotoGrapher::RealTimeFilming(CompressedFrame realtime_frame)
{
    //std::string msg = "������" + std::to_string(realtime_frame.GetFrames().size()) + "������\n";
    //printf(msg.c_str());
    concurrency_compressedframe_queue.AddQuestToQueue(std::make_unique<CompressedFrame>(std::move(realtime_frame)));
}

void PhotoGrapher::RecordFilmName(const std::string& film_name)
{
    current_film_name = film_name;
}

std::string PhotoGrapher::GetCurrentFilmName()
{
    return current_film_name;
}

CompressedFrame PhotoGrapher::TryGettingFrame()
{
    CompressedFrame nothing;
    if (concurrency_compressedframe_queue.Empty())
        return nothing;
    auto frame = concurrency_compressedframe_queue.GetQuestFromQueue();
    return *frame;
}
