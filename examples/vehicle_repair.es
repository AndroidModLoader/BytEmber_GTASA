#include "../include/aml-psdk/game_sa/entity/PlayerPed.esh"
#include "../include/aml-psdk/game_sa/entity/Vehicle.esh"

void onUpdate()
{
    CVehicle* vehicle = FindPlayerVehicle(-1, false);
    if(vehicle != NULL && vehicle.m_fHealth < 1000.0f) vehicle.m_fHealth = 1000.0f;
}

void main()
{
}
