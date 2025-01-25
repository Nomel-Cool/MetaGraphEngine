#include "FrameStructure.h"

std::vector<OnePixel> CompressedFrame::GetFrames() const
{
    return frames;
}

void CompressedFrame::UpdateFrames(const std::vector<OnePixel>& pixels)
{
    frames.clear();
    frames = pixels;
}