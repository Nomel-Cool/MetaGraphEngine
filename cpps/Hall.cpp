#include "Hall.h"

Hall::Hall(std::shared_ptr<FrameIDManager> shared_frame_id_manager)
{
    sp_frame_id_manager = shared_frame_id_manager;
}

const std::map<StagePos, std::shared_ptr<OnePixel>>& Hall::GetStage() const
{
    return stage;
}

std::map<StagePos, std::shared_ptr<OnePixel>>::iterator Hall::DeleteElementAt(const StagePos& pos)
{
    auto it = stage.find(pos);
    if (it != stage.end())
        return stage.erase(it);
    return stage.end();
}

bool Hall::TransferPixelFrom(const StagePos& coordinate_begin)
{
    if (stage.find(coordinate_begin) == stage.end())
        return false;

    std::shared_ptr<OnePixel> target_pixel = stage[coordinate_begin];

    target_pixel->cur_frame_id = sp_frame_id_manager->GetCurrentFrameID();

    //逻辑说明：
    //        优先执行单像素处理，因为它不会使自身像素增加，最多发生转移使其它像素复合，如果满足本if，则必然不进入复合处理的if
    //        如果一个复合像素执行了单像素声明，则它的里子会等待到满足复合处理后再处理
    // 
    //        如果发生了偏移且目的地 available 则直接占用
    //        如果发生了偏移且目的地 unavailable 则融合到已存在的像素内
    //        最终都要清理移动前的像素内存
    //        当对单像素强调为单像素：理由是可能其它复合像素或者单像素移动到自身导致自身成为复合像素
    //        当对单像素声明为复合像素是无效的，因为当其它像素融合进来后，它们已经固定了，对这部分执行复合像素处理是没有效果的，而如果不发生融合则更不应该执行复合像素处理
    if (target_pixel->owners_info.size() == 1 || target_pixel->GetSingleDeclaration())
    {
        if (target_pixel->GetSingleDeclaration())
            target_pixel->TryUpdatingInnersAccordingToSurface();

        auto& sp_dest_pixel = stage[{target_pixel->x, target_pixel->y, target_pixel->z}];
        if (sp_dest_pixel != target_pixel)
        {
            if (sp_dest_pixel != nullptr)
                sp_dest_pixel->Merge(target_pixel);
            else
                stage[{target_pixel->x, target_pixel->y, target_pixel->z}] = std::make_shared<OnePixel>(*target_pixel);
            stage[coordinate_begin] = nullptr; // 它将会被 TidyUp 函数识别并清理
        }
        else
            target_pixel->UpdateSurfaceByMainTag();
    }

    //逻辑说明：
    //        如果 seperate 出来的像素没有发生移动，则还原该分离，因为这只涉及到颜色的修改，由仍然有处理权的 target_pixel 来做即可
    //        如果 seperate 出来的像素移动到“空闲”位置，则直接占有
    //        否则 seperate 移动到了已经有人占有的位置，不应该对它有任何影响（颜色仍然由该位置原初占有者更新，受先后序影响，但原初占有者仍可以得到被共享的情况并做出变化，一帧的变化，肉眼无法察觉）
    //        对复合像素声明为单像素：如果没有更改surface坐标的前提下，该像素的单处理使其不变；否则将会发生批量把自身里子转移到其它像素位置
    if (target_pixel->owners_info.size() > 1 && !target_pixel->GetSingleDeclaration())
    {
        for (auto iter_owners_kv = target_pixel->owners_info.begin(); iter_owners_kv != target_pixel->owners_info.end();)
        {
            if (iter_owners_kv->second == nullptr)
            {
                ++iter_owners_kv;
                continue;
            }
            auto [it, inserted] = stage.try_emplace({ iter_owners_kv->second->x, iter_owners_kv->second->y, iter_owners_kv->second->z }, iter_owners_kv->second);
            if (!inserted)
            {
                if (iter_owners_kv->second->x != target_pixel->x || iter_owners_kv->second->y != target_pixel->y || iter_owners_kv->second->z != target_pixel->z)
                {
                    it->second->Merge(iter_owners_kv->second, true);
                    target_pixel->owners_info.erase(iter_owners_kv++); // 如果所有人全空，则会被TidyUp识别并清理
                }
                else
                    ++iter_owners_kv;
            }
            else
            {
                it->second->owners_info[it->second->tag] = iter_owners_kv->second;
                target_pixel->owners_info.erase(iter_owners_kv++);
            }
        }
        target_pixel->UpdateSurfaceByMainTag();
    }
    return true;
}

void Hall::PingStage(const std::vector<OnePixel>& ping_pixel_list, const std::size_t& graph_pos_in_list)
{
    for (const auto& ping_pixel : ping_pixel_list)
    {
        auto key = std::tie(ping_pixel.x, ping_pixel.y, ping_pixel.z);
        auto& sp_specific_pixel = stage[key];
        if (sp_specific_pixel)
        {
            OnePixel one_pixel;
            one_pixel.render_flag = true;
            one_pixel.x = ping_pixel.x;
            one_pixel.y = ping_pixel.y;
            one_pixel.z = ping_pixel.z;
            one_pixel.r = ping_pixel.r;
            one_pixel.g = ping_pixel.g;
            one_pixel.b = ping_pixel.b;
            one_pixel.a = ping_pixel.a;
            one_pixel.tag = ping_pixel.tag;
            one_pixel.block_size = ping_pixel.block_size;
            auto sync_sp_pixel = std::make_shared<OnePixel>(one_pixel);
            auto wrap_sync_sp_pixel = sync_sp_pixel->owners_info[sync_sp_pixel->tag] = std::make_shared<OnePixel>(*sync_sp_pixel);
            sp_specific_pixel->Merge(sync_sp_pixel);
            checkin_sequence[graph_pos_in_list].push(wrap_sync_sp_pixel);
        }
        else
        {
            // 基础设置
            OnePixel one_pixel;
            one_pixel.render_flag = true;
            one_pixel.x = ping_pixel.x;
            one_pixel.y = ping_pixel.y;
            one_pixel.z = ping_pixel.z;
            one_pixel.r = ping_pixel.r;
            one_pixel.g = ping_pixel.g;
            one_pixel.b = ping_pixel.b;
            one_pixel.a = ping_pixel.a;
            one_pixel.tag = ping_pixel.tag;
            one_pixel.block_size = ping_pixel.block_size;
            auto sync_sp_pixel = std::make_shared<OnePixel>(one_pixel);
            auto wrap_sync_sp_pixel = sync_sp_pixel->owners_info[sync_sp_pixel->tag] = std::make_shared<OnePixel>(*sync_sp_pixel);
            sp_specific_pixel = sync_sp_pixel;
            checkin_sequence[graph_pos_in_list].push(wrap_sync_sp_pixel);
        }
    }
}

std::map<std::size_t, std::vector<OnePixel>> Hall::CollectStage()
{
    std::map<std::size_t, std::vector<OnePixel>> collected_pixels;
    for (auto& sequencial_pixels : checkin_sequence)
    {
        while (!sequencial_pixels.second.empty())
        {
            collected_pixels[sequencial_pixels.first].emplace_back(*(sequencial_pixels.second.front()));
            sequencial_pixels.second.pop();
        }
    }
    checkin_sequence.clear();
    return collected_pixels;
}
