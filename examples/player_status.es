#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"
#include "../include/aml-psdk/game_sa/base/Timer.esh"

uint32_t nextReport = 0;

void onUpdate()
{
    if(!CTimer::IsTimePassed(nextReport)) return;
    nextReport = CTimer::GetTimeMS() + 5000;
    CPlayerPed* player = FindPlayerPed(-1);
    if(player != NULL)
    {
        printf("Player health: %.1f / %.1f, armour: %.1f, standing: %d\n",
               player.GetHealth(), player.GetMaxHealth(), player.m_fArmour,
               (int)player.bIsStanding);
    }
}

void main()
{
}
