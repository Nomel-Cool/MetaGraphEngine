#pragma once
#ifndef FRAME_GENERATE_INTERVAL_H
#define FRAME_GENERATE_INTERVAL_H

class FrameGenerateIntervalManager
{
public:
	void SetFrameGenerationInterval(int interval);
	int GetFrameGenerationInterval() const;
private:
	int frame_generation_interval = 18;//ms 这个数与实时渲染控制帧生成的速度，不是帧动画渲染的直接帧数
};

#endif // !FRAME_GENERATE_INTERVAL_H
