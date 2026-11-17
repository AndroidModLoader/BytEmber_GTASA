require float GetPlayerArmour() from "player_lib";
require void SetPlayerArmour(float armour) from "player_lib";

void main()
{
    while(true)
    {
        wait(1000);
        float armour = GetPlayerArmour();
        if(armour < 0.0f || armour >= 100.0f) continue;
        armour = armour + 1.0f;
        if(armour > 100.0f) armour = 100.0f;
        SetPlayerArmour(armour);
    }
}
