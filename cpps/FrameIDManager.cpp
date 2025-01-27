#include "FrameIDManager.h"

const uint64_t& FrameIDManager::GetCurrentFrameID() const
{
    return frame_id;
}

void FrameIDManager::NextFrame()
{
    ++frame_id;
}
