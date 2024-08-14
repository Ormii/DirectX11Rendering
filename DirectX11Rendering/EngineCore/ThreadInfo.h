#pragma once

extern thread_local uint32 LthreadID;

struct ThreadParam
{
	int commandListIdx;
	float deltatime;
};