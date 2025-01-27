#include "FrameGenerateIntervalManager.h"

void FrameGenerateIntervalManager::SetFrameGenerationInterval(int interval)
{
	frame_generation_interval = interval;
}

int FrameGenerateIntervalManager::GetFrameGenerationInterval() const
{
	return frame_generation_interval;
}
