#pragma once
#ifndef HALL_H
#define HALL_H

#include <tuple>
#include <vector>
#include <map>
#include <memory>
#include <queue>
#include <mutex>

#include "PixelType.h"
#include "FrameIDManager.h"

/// <summary>
/// ������̨���֣�ͼԪ��������̨��Ӱ����̨����
/// </summary>
class Hall
{
public:
	Hall() = default;
	Hall(std::shared_ptr<FrameIDManager> shared_frame_id_manager);
	const std::map<StagePos, std::shared_ptr<OnePixel>>& GetStage() const;
	std::map<StagePos, std::shared_ptr<OnePixel>>::iterator DeleteElementAt(const StagePos& pos);

	/// <summary>
	/// ֪ͨ�ƽ���������Ȩ�����������ʵ�ֿ�����ֵ����
	/// </summary>
	/// <typeparam name="OnePixel"></typeparam>
	/// <param name="coordinate_begin"></param>
	/// <param name="coordinate_end"></param>
	/// <returns></returns>
	bool TransferPixelFrom(const StagePos& coordinate_begin);

	void PingStage(const std::vector<OnePixel>& pixel_list, const std::size_t& graph_pos_in_list);
	std::map<std::size_t, std::vector<OnePixel>> CollectStage();

protected:
private:
	std::map<StagePos, std::shared_ptr<OnePixel>> stage;
	std::map<std::size_t, std::queue<std::shared_ptr<OnePixel>>> checkin_sequence; // <graph_id, {pixel1,...,pixeln}>
	std::shared_ptr<FrameIDManager> sp_frame_id_manager;
	std::mutex stage_lock;
};

#endif //HALL_H 
