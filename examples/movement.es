#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"

void main()
{
    CPlayerPed* player = FindPlayerPed(0);
    if(player == NULL || !player.IsAlive()) return;

    // m_vecMoveSpeed is a borrowed view into CPhysical, not a copy.
    player.m_vecMoveSpeed.x = 0.0f;
    player.m_vecMoveSpeed.y = 0.0f;
    player.m_vecMoveSpeed.z = 0.05f;
}
