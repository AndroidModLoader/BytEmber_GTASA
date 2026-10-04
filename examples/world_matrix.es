#include "../include/aml-psdk/game_sa/entity/Ped.esh"

RwMatrix* GetBoneMatrix(RwFrame* frame, RwMatrix* destination)
{
    return CPedIK::GetWorldMatrix(frame, destination);
}

void main()
{
    // GetBoneMatrix can be called by your host with valid, borrowed frame/matrix handles.
}
