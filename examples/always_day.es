#include "../include/aml-psdk/game_sa/other/Clock.esh"

void main()
{
    while(true)
    {
        if(!CClock::GetIsTimeInRange(7, 20)) CClock::SetGameClock(12, 0, 0);
        wait(2000);
    }
}
