#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"

void onUpdate()
{
    CPlayerPed* player = FindPlayerPed(0);
    if(player == NULL) return;

    // Fixed arrays and their elements are borrowed views into game memory.
    for(uint64_t i = 0; i < player.m_aWeapons.length; ++i)
    {
        CWeapon* weapon = player.m_aWeapons[i];
        if(weapon.m_eWeaponType != WEAPONTYPE_UNARMED && weapon.m_nAmmoTotal < 500)
        {
            weapon.m_nAmmoTotal = 500;
        }
    }
}

void main()
{
}
