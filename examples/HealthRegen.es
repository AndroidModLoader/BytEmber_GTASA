// Port of aml-psdk/examples/game_sa/health_regen/main.cpp.
// Call onUpdate once per GTA gameProcessEvent frame.
#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"
#include "../include/aml-psdk/game_sa/base/Timer.esh"

void onUpdate()
{
    CPlayerPed* player = FindPlayerPed(-1);
    if(player == NULL || !player.IsAlive() || player.IsMaxHealth()) return;

    if(CTimer::IsTimePassed(player.m_nLastDamagedTime + 7000))
    {
        float hpPerFrame = CTimer::GetTimeScale() * CTimer::GetTimeStepInSeconds() * 1.0f;
        player.AddHealthSafe(hpPerFrame);
    }
}

void main()
{
}
