#include <mod/amlmod.h>
#include <mod/logger.h>

MYMOD(net.rusjj.bytember.sa, BytEmber (SA Version), 1.0, RusJJ)

#include "ibytember.sa.h"
struct BytEmberSA : public IBytEmberSA
{
    void Reserved1(){}
    void Reserved2(){}
    void Reserved3(){}
    void Reserved4(){}
    void Reserved5(){}
    void Reserved6(){}
    void Reserved7(){}
    void Reserved8(){}
};
static BytEmberSA bytember;

void *hGame;
uintptr_t pGame;

ON_MOD_PRELOAD()
{
    logger->SetTag("BytEmber");
    RegisterInterface("BytEmber.SA", &bytember);

    pGame = aml->GetLib("libGTASA.so");
    hGame = aml->GetLibHandle("libGTASA.so");

    if(!pGame || !hGame)
    {
        logger->Error("Failed to get game library");
        return;
    }

    // da job.
}