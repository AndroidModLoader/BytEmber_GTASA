#include <mod/amlmod.h>
#include <mod/logger.h>

#include "game_sa.h"
#include "script_host.h"
#include <chrono>

namespace
{
    ScriptHost* scripts = nullptr;
    bool attached = false, gameplayTicked = false, clockStarted = false;
    unsigned int frameDepth = 0, processDepth = 0, transitionDepth = 0;
    uint32_t* gameMilliseconds = nullptr;
    uint32_t lastGameMilliseconds = 0;
    uint64_t gameplayMilliseconds = 0;

    struct DepthGuard
    {
        unsigned int& depth;
        DepthGuard(unsigned int& value) : depth(value) { ++depth; }
        ~DepthGuard() { --depth; }
    };

    uint64_t GlobalMilliseconds()
    {
        return uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    void ResetSession()
    {
        if(!attached) return;
        scripts->ResetGameplay();
        clockStarted = false;
        gameplayMilliseconds = 0;
    }

    void GlobalFrame()
    {
        if(attached && frameDepth == 1) scripts->GlobalFrame(GlobalMilliseconds());
    }

    // Idle's arguments are selected by the exported C++ signature, rather than
    // by offsets or assumptions about the architecture. SA 2.10 exports Pvb.
    // The Pv variant can be used only if the game actually exports that name.
    DECL_HOOKv(GameIdle, void* param, bool firstFrame)
    {
        DepthGuard guard(frameDepth);
        if(frameDepth == 1) gameplayTicked = false;
        GameIdle(param, firstFrame);
        GlobalFrame();
    }

    DECL_HOOKv(GameIdleLegacy, void* param)
    {
        DepthGuard guard(frameDepth);
        if(frameDepth == 1) gameplayTicked = false;
        GameIdleLegacy(param);
        GlobalFrame();
    }

    DECL_HOOKv(FrontendIdle)
    {
        DepthGuard guard(frameDepth);
        FrontendIdle();
        GlobalFrame();
    }

    DECL_HOOKv(GameProcess)
    {
        DepthGuard guard(processDepth);
        GameProcess();
    }

    DECL_HOOKv(ScriptsProcess)
    {
        ScriptsProcess();
        // The game invokes Process during initialization too. Run entries only
        // inside an ordinary Idle -> CGame::Process frame, once per outer frame.
        // CGame's own pause checks gate this call; no MobileMenu layout is needed.
        if(!attached || frameDepth != 1 || processDepth != 1 || transitionDepth || gameplayTicked) return;
        gameplayTicked = true;
        uint32_t now = *gameMilliseconds;
        if(clockStarted) gameplayMilliseconds += uint32_t(now - lastGameMilliseconds);
        lastGameMilliseconds = now;
        clockStarted = true;
        scripts->GameplayFrame(gameplayMilliseconds);
    }

    DECL_HOOKv(ScriptsInit)
    {
        DepthGuard guard(transitionDepth);
        ResetSession();
        ScriptsInit();
    }

    DECL_HOOK(bool, GenericLoad, bool& wrongVersion)
    {
        DepthGuard guard(transitionDepth);
        ResetSession();
        return GenericLoad(wrongVersion);
    }

    DECL_HOOK(bool, GameShutdown)
    {
        DepthGuard guard(transitionDepth);
        ResetSession();
        return GameShutdown();
    }

    DECL_HOOKv(ShutdownForRestart)
    {
        DepthGuard guard(transitionDepth);
        ResetSession();
        ShutdownForRestart();
    }

    uintptr_t Symbol(void* game, const char* name)
    {
        uintptr_t address = aml->GetSym(game, name);
        if(!address) logger->Error("Required game symbol missing: %s", name);
        return address;
    }
}

bool AttachGameSA(void* game, ScriptHost* host)
{
    // Resolve everything before altering any entry. On a missing symbol, leave
    // the game alone. Hook failures disable scheduling; installed hooks forward.
    uintptr_t idle = aml->GetSym(game, "_Z4IdlePvb");
    bool legacyIdle = !idle;
    if(legacyIdle) idle = Symbol(game, "_Z4IdlePv");
    uintptr_t frontend = Symbol(game, "_Z12FrontendIdlev");
    uintptr_t process = Symbol(game, "_ZN5CGame7ProcessEv");
    uintptr_t scriptProcess = Symbol(game, "_ZN11CTheScripts7ProcessEv");
    uintptr_t scriptInit = Symbol(game, "_ZN11CTheScripts4InitEv");
    uintptr_t load = Symbol(game, "_ZN19CGenericGameStorage11GenericLoadERb");
    uintptr_t shutdown = Symbol(game, "_ZN5CGame8ShutdownEv");
    uintptr_t restart = Symbol(game, "_ZN5CGame18ShutDownForRestartEv");
    uintptr_t time = Symbol(game, "_ZN6CTimer22m_snTimeInMillisecondsE");
    if(!idle || !frontend || !process || !scriptProcess || !scriptInit ||
       !load || !shutdown || !restart || !time) return false;

    scripts = host;
    gameMilliseconds = (uint32_t*)time;
    if(!(legacyIdle ? HOOK(GameIdleLegacy, idle) : HOOK(GameIdle, idle)) ||
       !HOOK(FrontendIdle, frontend) || !HOOK(GameProcess, process) ||
       !HOOK(ScriptsProcess, scriptProcess) || !HOOK(ScriptsInit, scriptInit) ||
       !HOOK(GenericLoad, load) || !HOOK(GameShutdown, shutdown) ||
       !HOOK(ShutdownForRestart, restart))
    {
        logger->Error("Failed to install game hooks; script scheduling disabled");
        return false;
    }
    attached = true;
    return true;
}
