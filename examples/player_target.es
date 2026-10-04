#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"
#include "../include/aml-psdk/game_sa/base/Timer.esh"

void onUpdate()
{
    CPlayerPed* player = FindPlayerPed(0);
    if(player == NULL) return;

    CPed* target = player.m_pPlayerTargettedPed;
    if(target != NULL && target.IsAlive())
    {
        float hpPerFrame = CTimer::GetTimeScale() * CTimer::GetTimeStepInSeconds() * 10.0f;
        target.AddHealthSafe(hpPerFrame);
        target.m_fArmour = 100.0f;
    }
}

void main()
{
}
