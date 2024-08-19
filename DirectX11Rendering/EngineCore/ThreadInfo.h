#pragma once

extern thread_local uint32 LthreadID;

struct ThreadParam
{
	uint32 threadID;
	uint32 commandListIdx;
	float deltatime;

	uint32 startIdx;
	uint32 endIdx;
};