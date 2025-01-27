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

    //�߼�˵����
    //        ����ִ�е����ش�����Ϊ������ʹ�����������ӣ���෢��ת��ʹ�������ظ��ϣ�������㱾if�����Ȼ�����븴�ϴ����if
    //        ���һ����������ִ���˵��������������������ӻ�ȴ������㸴�ϴ�����ٴ���
    // 
    //        ���������ƫ����Ŀ�ĵ� available ��ֱ��ռ��
    //        ���������ƫ����Ŀ�ĵ� unavailable ���ںϵ��Ѵ��ڵ�������
    //        ���ն�Ҫ�����ƶ�ǰ�������ڴ�
    //        ���Ե�����ǿ��Ϊ�����أ������ǿ��������������ػ��ߵ������ƶ��������������Ϊ��������
    //        ���Ե���������Ϊ������������Ч�ģ���Ϊ�����������ںϽ����������Ѿ��̶��ˣ����ⲿ��ִ�и������ش�����û��Ч���ģ�������������ں������Ӧ��ִ�и������ش���
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
            stage[coordinate_begin] = nullptr; // �����ᱻ TidyUp ����ʶ������
        }
        else
            target_pixel->UpdateSurfaceByMainTag();
    }

    //�߼�˵����
    //        ��� seperate ����������û�з����ƶ�����ԭ�÷��룬��Ϊ��ֻ�漰����ɫ���޸ģ�����Ȼ�д���Ȩ�� target_pixel ��������
    //        ��� seperate �����������ƶ��������С�λ�ã���ֱ��ռ��
    //        ���� seperate �ƶ������Ѿ�����ռ�е�λ�ã���Ӧ�ö������κ�Ӱ�죨��ɫ��Ȼ�ɸ�λ��ԭ��ռ���߸��£����Ⱥ���Ӱ�죬��ԭ��ռ�����Կ��Եõ������������������仯��һ֡�ı仯�������޷������
    //        �Ը�����������Ϊ�����أ����û�и���surface�����ǰ���£������صĵ�����ʹ�䲻�䣻���򽫻ᷢ����������������ת�Ƶ���������λ��
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
                    target_pixel->owners_info.erase(iter_owners_kv++); // ���������ȫ�գ���ᱻTidyUpʶ������
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
            // ��������
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
