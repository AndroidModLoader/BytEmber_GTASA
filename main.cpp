#include <mod/amlmod.h>
#include <mod/logger.h>

#include <ember/stdlib.hpp>
#include "game_sa.h"
#include "script_host.h"
#include "include/psdk_calls.h"

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
static std::unique_ptr<ScriptHost> scripts;

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
}

ON_ALL_MODS_LOAD()
{
    if(!pGame || !hGame) return;
    const char* dataPath = aml->GetAndroidDataRootPath();
    if(!dataPath || !*dataPath)
    {
        logger->Error("Failed to get Android game data directory");
        return;
    }

    try
    {
        std::string scriptPath = std::string(dataPath) + "/bytember";
        std::string globalPath = scriptPath + "/global";
        if(!aml->IsDirectory(scriptPath.c_str()) && !aml->CreateDirRecursive(scriptPath.c_str()))
        {
            logger->Error("Failed to create script directory: %s", scriptPath.c_str());
            return;
        }
        if(!aml->IsDirectory(globalPath.c_str()) && !aml->CreateDirRecursive(globalPath.c_str()))
        {
            logger->Error("Failed to create global script directory: %s", globalPath.c_str());
            return;
        }

        ember::Registry registry(false);
        ember::StdOptions options;
        options.output = [](const std::string& message) { logger->Info("%s", message.c_str()); };
        ember::EMBER_RegisterStd(registry, std::move(options));
        scripts.reset(new ScriptHost(scriptPath, std::move(registry),
            [](bool error, const std::string& message)
            {
                if(error) logger->Error("%s", message.c_str());
                else logger->Info("%s", message.c_str());
            }, PSDK::Calls()));
        if(!AttachGameSA(hGame, scripts.get())) return;
        logger->Info("Attached; loading bytecode from %s", scriptPath.c_str());
    }
    catch(const std::exception& error)
    {
        logger->Error("Failed to attach BytEmber: %s", error.what());
    }
}
