#include "FilmStorage.h"

void FilmNameMap::CollectPixelStream(const OnePixel& pixel)
{
    film_cache.emplace_back(pixel);
}

void FilmNameMap::Store(const std::string& film_name = "")
{
    CompressedFrame compressed_frame;
    compressed_frame.UpdateFrames(film_cache);
    if (film_storage.find(film_name) == film_storage.end())
        film_storage.insert(std::make_pair(film_name, std::move(compressed_frame)));
    else
        film_storage[film_name].UpdateFrames(film_cache);
    film_cache.clear();
}

CompressedFrame FilmNameMap::Fetch(const std::string& film_name)
{
    if (film_name.empty() || film_storage.find(film_name) == film_storage.end())
        return {};
    return film_storage[film_name];
}

void FilmNameMap::ClearMemo()
{
    film_cache.clear();
    film_storage.clear();
}

void TaskModelFrameQueue::CollectPixelStream(const OnePixel& pixel)
{
    film_cache.emplace_back(pixel);
}

void TaskModelFrameQueue::Store(const std::string& film_name = "")
{
    CompressedFrame realtime_frame;
    realtime_frame.UpdateFrames(film_cache);
    concurrency_compressedframe_queue.AddQuestToQueue(std::make_unique<CompressedFrame>(std::move(realtime_frame)));
    film_cache.clear();
}

CompressedFrame TaskModelFrameQueue::Fetch(const std::string& film_name = "")
{
    CompressedFrame nothing;
    if (concurrency_compressedframe_queue.Empty())
        return nothing;
    auto frame = concurrency_compressedframe_queue.GetQuestFromQueue();
    return *frame;
}

void TaskModelFrameQueue::ClearMemo()
{
    concurrency_compressedframe_queue.ClearQueue();
}
