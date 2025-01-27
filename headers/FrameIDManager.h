#pragma once
#ifndef FRAME_ID_MANAGER_H
#define FRAME_ID_MANAGER_H

#include <cstdint>
class FrameIDManager
{
public:
	const uint64_t& GetCurrentFrameID() const;
	void NextFrame();
private:
	uint64_t frame_id = 1;
};

#endif // !FRAME_ID_MANAGER_H
