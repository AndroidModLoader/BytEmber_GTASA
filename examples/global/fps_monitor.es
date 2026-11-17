#include "../../include/aml-psdk/game_sa/base/Timer.esh"

void main()
{
    while(true)
    {
        wait(10000);
        printf("FPS: %.1f, paused: %d\n", CTimer::game_FPS(), (int)CTimer::IsPaused());
    }
}
