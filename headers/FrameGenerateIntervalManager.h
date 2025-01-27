#pragma once
#ifndef FRAME_GENERATE_INTERVAL_H
#define FRAME_GENERATE_INTERVAL_H

class FrameGenerateIntervalManager
{
public:
	void SetFrameGenerationInterval(int interval);
	int GetFrameGenerationInterval() const;
private:
	int frame_generation_interval = 18;//ms �������ʵʱ��Ⱦ����֡���ɵ��ٶȣ�����֡������Ⱦ��ֱ��֡��
};

#endif // !FRAME_GENERATE_INTERVAL_H
