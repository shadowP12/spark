#pragma once

#include <map>
#include <string>
#include <vector>

namespace sp
{
class ModuleInterface
{
public:
    virtual void Startup() {}

    virtual void Shutdown() {}
};

template<class T>
class Module : public ModuleInterface
{
public:
    static T* Get()
    {
        static T _instance;
        return &_instance;
    }
};

class ModuleManager
{
public:
    static ModuleManager* Get()
    {
        static ModuleManager _instance;
        return &_instance;
    }

    void StartupAllModule();
    void ShutdownAllModule();

    std::vector<std::string> launch_list;
    std::map<std::string, ModuleInterface*> module_table;
    std::map<std::string, std::vector<std::string>> module_dependency;
};

class ModuleRegisterFunction
{
public:
    ModuleRegisterFunction(const std::string& module_name, ModuleInterface* module_inst)
    {
        ModuleManager::Get()->module_table[module_name] = module_inst;
    }
};

class ModuleDependencyFunction
{
public:
    ModuleDependencyFunction(const std::string& module_name, const std::string& dep_name)
    {
        ModuleManager::Get()->module_dependency[module_name].push_back(dep_name);
    }
};
}// namespace sp

#define SPARK_MODULE_REGISTER(x) \
    sp::ModuleRegisterFunction Register_##x(#x, sp::x::Get());

#define SPARK_MODULE_DEPENDENCY(x, d) \
    sp::ModuleDependencyFunction Dependency_##x##d(#x, #d);