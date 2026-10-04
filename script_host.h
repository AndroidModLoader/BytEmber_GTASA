#pragma once

#include <ember/runtime.hpp>

// The scheduler does not depend on GTA addresses or AML. Only the engine adapter
// decides when a frame is gameplay and when its VMs must be destroyed.
class ScriptHost
{
    struct Group;
    std::unique_ptr<Group> global, gameplay;
    ember::Registry registry;
    ember::NativeOptions native;
    std::string directory, globalDirectory;
    std::function<void(bool, const std::string&)> log;
    bool running = false, resetGameplay = false;

    void Tick(std::unique_ptr<Group>& group, const std::string& path, uint64_t milliseconds);

public:
    ScriptHost(std::string path, ember::Registry natives,
               std::function<void(bool, const std::string&)> logger,
               ember::NativeOptions options = {});
    ~ScriptHost();
    void GlobalFrame(uint64_t milliseconds);
    void GameplayFrame(uint64_t milliseconds);
    void ResetGameplay();
};
