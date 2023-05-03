#include <windows.h>
#include <stdint.h>

uint64_t Timer_GetTicks()
{
	uint64_t ticks;

	QueryPerformanceCounter((LARGE_INTEGER*)&ticks);

	return ticks;
}
uint64_t Timer_GetFrequency()
{
	uint64_t freq;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

	return freq;
}
double Timer_TicksToSecondsf64(uint64_t ticks)
{
	return ticks / (double)Timer_GetFrequency();
}
float Timer_TicksToSecondsf32(uint64_t ticks)
{
	return (float)(ticks / (double)Timer_GetFrequency());
}
