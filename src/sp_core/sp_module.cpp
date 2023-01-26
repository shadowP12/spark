#include "sp_module.h"
#include <queue>

namespace sp
{
void ModuleManager::StartupAllModule()
{
    // BFS
    std::queue<std::string> bfs;
    std::map<std::string, bool> visited;
    for (const auto& module : module_table)
    {
        visited[module.first] = false;
        if (module_dependency.find(module.first) == module_dependency.end())
        {
            visited[module.first] = true;
            bfs.push(module.first);
        }
    }

    while (!bfs.empty())
    {
        launch_list.push_back(bfs.front());
        bfs.pop();
        for (const auto& module_dep : module_dependency)
        {
            if (visited[module_dep.first])
                continue;
            bool is_ready = true;
            for (const auto& dep : module_dep.second)
            {
                if (!visited[dep])
                {
                    is_ready = false;
                }
            }
            if (is_ready)
            {
                visited[module_dep.first] = true;
                bfs.push(module_dep.first);
            }
        }
    }

    for (int i = 0; i < launch_list.size(); ++i)
        module_table[launch_list[i]]->Startup();
}

void ModuleManager::ShutdownAllModule()
{
    for (int i = launch_list.size() - 1; i >= 0; --i)
        module_table[launch_list[i]]->Shutdown();
}
}// namespace sp