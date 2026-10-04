#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"
#include "../include/aml-psdk/game_sa/base/Timer.esh"

uint32_t nextHeal = 0;

void onUpdate()
{
    if(!CTimer::IsTimePassed(nextHeal)) return;
    nextHeal = CTimer::GetTimeMS() + 1000;
    CPlayerPed* player = FindPlayerPed(-1);
    if(player != NULL && player.IsAlive())
    {
        player.SetHealthSafe(player.GetMaxHealth());
    }
}

void main()
{
}
