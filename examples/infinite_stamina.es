#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"
#include "../include/aml-psdk/game_sa/other/PlayerData.esh"
#include "../include/aml-psdk/game_sa/other/Stats.esh"

void onUpdate()
{
    CPlayerPed* player = FindPlayerPed(-1);
    if(player == NULL || !player.IsAlive()) return;

    CPlayerData* data = player.m_pPlayerData;
    if(data == NULL) return;
    data.m_fTimeCanRun = CStats::GetFatAndMuscleModifier(STAT_MOD_TIME_CAN_RUN);
    data.m_fBreath = CStats::GetFatAndMuscleModifier(STAT_MOD_AIR_IN_LUNG);
}

void main()
{
}
