#include "script_host.h"

#include <algorithm>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
#else
    #include <dirent.h>
#endif

namespace
{
    const size_t MAX_SCRIPT_BYTES = 16 * 1024 * 1024;
    const uint64_t FRAME_INSTRUCTIONS = 10000;

    bool ValidName(const std::string& name)
    {
        return !name.empty() && name != "." && name != ".." &&
               name.find_first_of("/\\:") == std::string::npos;
    }

    ember::Program ReadProgram(const std::string& path)
    {
        FILE* file = fopen(path.c_str(), "rb");
        if(!file) throw ember::Error("cannot open bytecode: " + path);
        struct FileGuard
        {
            FILE* file;
            ~FileGuard() { fclose(file); }
        } guard{file};
#ifdef _WIN32
        struct _stat64 info;
        if(_fstat64(_fileno(file), &info) || (info.st_mode & _S_IFMT) != _S_IFREG)
#else
        struct stat info;
        if(fstat(fileno(file), &info) || !S_ISREG(info.st_mode))
#endif
            throw ember::Error("cannot stat bytecode file: " + path);
        if(info.st_size <= 0 || uint64_t(info.st_size) > MAX_SCRIPT_BYTES)
            throw ember::Error("invalid bytecode file or size: " + path);
        std::vector<uint8_t> bytes((size_t)info.st_size);
        if(fread(bytes.data(), 1, bytes.size(), file) != bytes.size())
            throw ember::Error("cannot read bytecode: " + path);
        return ember::Program::load(bytes.data(), bytes.size(), MAX_SCRIPT_BYTES);
    }
}

struct ScriptHost::Group
{
    struct Entry
    {
        std::string name;
        ember::VM* vm;
        ember::Value update;
        bool mainActive;
    };

    std::string path;
    uint64_t milliseconds = 0;
    ember::ScriptRuntime runtime;
    std::vector<Entry> entries;
    std::function<void(bool, const std::string&)> log;

    Group(const std::string& directory, const ember::Registry& registry,
          std::function<void(bool, const std::string&)> logger,
          const ember::NativeOptions& native) :
        path(directory), runtime(registry, [this](const std::string& name)
        {
            if(!ValidName(name)) throw ember::Error("invalid script name: " + name);
            return ReadProgram(path + "/" + name + ".ebc");
        }, {}, native), log(std::move(logger))
    {
    }

    void Load()
    {
        std::vector<std::string> names;
#ifdef _WIN32
        WIN32_FIND_DATAA entry;
        HANDLE dir = FindFirstFileA((path + "/*").c_str(), &entry);
        if(dir == INVALID_HANDLE_VALUE)
        {
            DWORD error = GetLastError();
            if(error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
                log(true, "cannot scan " + path + ": error " + std::to_string(error));
            return;
        }
        struct DirectoryGuard
        {
            HANDLE dir;
            ~DirectoryGuard() { FindClose(dir); }
        } guard{dir};
        do
        {
            if(entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            std::string name(entry.cFileName);
            if(name.size() <= 4 || name.compare(name.size() - 4, 4, ".ebc")) continue;
            name.resize(name.size() - 4);
            if(ValidName(name)) names.push_back(name);
        }
        while(FindNextFileA(dir, &entry));
        if(GetLastError() != ERROR_NO_MORE_FILES)
            log(true, "cannot finish scanning " + path);
#else
        DIR* dir = opendir(path.c_str());
        if(!dir)
        {
            if(errno != ENOENT) log(true, "cannot scan " + path + ": " + strerror(errno));
            return;
        }
        struct DirectoryGuard
        {
            DIR* dir;
            ~DirectoryGuard() { closedir(dir); }
        } guard{dir};
        while(true)
        {
            errno = 0;
            struct dirent* entry = readdir(dir);
            if(!entry)
            {
                if(errno) log(true, "cannot finish scanning " + path + ": " + strerror(errno));
                break;
            }
            std::string name(entry->d_name);
            if(name.size() <= 4 || name.compare(name.size() - 4, 4, ".ebc")) continue;
            struct stat info;
            if(stat((path + "/" + name).c_str(), &info) || !S_ISREG(info.st_mode)) continue;
            name.resize(name.size() - 4);
            if(ValidName(name)) names.push_back(name);
        }
#endif
        std::sort(names.begin(), names.end());
        for(const auto& name : names)
        {
            try
            {
                runtime.load(name);
                log(false, "loaded " + path + "/" + name + ".ebc");
            }
            catch(const std::exception& error)
            {
                log(true, path + "/" + name + ": " + error.what());
            }
        }
        // Load every module before starting entries, so optional imports do not
        // depend on which main() happens to be scheduled first.
        for(const auto& name : names)
        {
            ember::VM* vm = runtime.find(name);
            if(!vm || vm->status() == ember::Status::Failed) continue;
            try
            {
                vm->setClock([this] { return milliseconds; });
                const auto& functions = vm->program().functions;
                bool hasMain = false;
                ember::Value update;
                for(uint32_t i = 0; i < functions.size(); ++i)
                {
                    const ember::Function& function = functions[i];
                    if(function.name == "main") hasMain = true;
                    else if(function.name == "onUpdate")
                    {
                        if(function.result.kind != ember::Kind::Void || !function.params.empty())
                        {
                            log(true, path + "/" + name + ": expected void onUpdate()");
                            continue;
                        }
                        update.tag = ember::Value::Function;
                        update.bits = i + 1;
                    }
                }
                if(!hasMain && update.tag != ember::Value::Function) continue;
                if(hasMain) vm->start();
                entries.push_back({name, vm, update, hasMain});
            }
            catch(const std::exception& error)
            {
                log(true, path + "/" + name + ": " + error.what());
            }
        }
    }

    void Frame(const bool& reset)
    {
        const std::vector<ember::Value> noArgs;
        for(auto it = entries.begin(); it != entries.end();)
        {
            if(reset) break; // A native callback may request a session reset.
            ember::VM* vm = it->vm;
            try
            {
                ember::Status state = vm->status();
                if(it->mainActive)
                {
                    state = vm->run(FRAME_INSTRUCTIONS);
                    if(state == ember::Status::Finished) it->mainActive = false;
                }
                if(state == ember::Status::Failed)
                {
                    log(true, path + "/" + it->name + ": " + vm->error());
                    it = entries.erase(it);
                    continue;
                }
                if(reset) break;
                if(it->update.tag == ember::Value::Function) vm->invoke(it->update, noArgs);
                if(!it->mainActive && it->update.tag != ember::Value::Function) it = entries.erase(it);
                else ++it;
            }
            catch(const std::exception& error)
            {
                log(true, path + "/" + it->name + ": " + error.what());
                it = entries.erase(it);
            }
        }
    }
};

ScriptHost::ScriptHost(std::string path, ember::Registry natives,
                       std::function<void(bool, const std::string&)> logger,
                       ember::NativeOptions options) :
    registry(std::move(natives)), native(std::move(options)), directory(std::move(path)),
    globalDirectory(directory + "/global"), log(std::move(logger))
{
}

ScriptHost::~ScriptHost() = default;

void ScriptHost::Tick(std::unique_ptr<Group>& group, const std::string& path, uint64_t milliseconds)
{
    if(running) return; // Loading-screen pumps and future native callbacks can reenter.
    running = true;
    struct Guard { bool& flag; ~Guard() { flag = false; } } guard{running};
    try
    {
        if(!group)
        {
            group.reset(new Group(path, registry, log, native));
            group->milliseconds = milliseconds;
            group->Load();
        }
        group->milliseconds = milliseconds;
        const bool neverReset = false;
        if(&group == &gameplay) group->Frame(resetGameplay);
        else group->Frame(neverReset);
    }
    catch(const std::exception& error)
    {
        log(true, path + ": " + error.what());
    }
    if(resetGameplay) gameplay.reset(); // Destroy only after VM/native frames unwind.
}

void ScriptHost::GlobalFrame(uint64_t milliseconds)
{
    Tick(global, globalDirectory, milliseconds);
}

void ScriptHost::GameplayFrame(uint64_t milliseconds)
{
    if(running) return;
    if(resetGameplay)
    {
        gameplay.reset();
        resetGameplay = false;
    }
    Tick(gameplay, directory, milliseconds);
}

void ScriptHost::ResetGameplay()
{
    resetGameplay = true;
    if(!running) gameplay.reset();
}
