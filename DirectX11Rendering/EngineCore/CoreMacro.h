#pragma once

/*
	Crash
*/

#define CRASH(name)								\
{												\
	uint32* crash = nullptr;					\
	__analysis_assume(crash != nullptr);		\
	*crash = 0xDEADBEEF	;						\
}												\