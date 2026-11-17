#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"

// Exports only; loaded by `require ... from "player_lib"` in the same directory.
export float GetPlayerArmour()
{
    CPlayerPed* player = FindPlayerPed(-1);
    if(player == NULL || !player.IsAlive()) return -1.0f;
    return player.m_fArmour;
}

export void SetPlayerArmour(float armour)
{
    CPlayerPed* player = FindPlayerPed(-1);
    if(player != NULL && player.IsAlive()) player.m_fArmour = armour;
}
