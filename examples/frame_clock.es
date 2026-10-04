#include "../include/aml-psdk/game_sa/base/Timer.esh"

uint32_t nextReport = 0;
uint64_t frames = 0;

void onUpdate()
{
    ++frames;
    if(!CTimer::IsTimePassed(nextReport)) return;
    printf("Game frames: %llu, time: %u ms, step: %.4f seconds\n",
           frames, CTimer::GetTimeMS(), CTimer::GetTimeStepInSeconds());
    nextReport = CTimer::GetTimeMS() + 5000;
}

void main()
{
}
