#include "config.h"
#include "island_EA.h"

#include <sstream>
#include <string>

char* getParam(char * needle, char* haystack[], int count) 
{
    int i = 0;
    for (i = 0; i < count; i++) 
    {
        if (strcmp(needle, haystack[i]) == 0) 
        {
            if (i < count - 1)
            {
                return haystack[i + 1];
            }
        }
    }
    return 0;
}

int generateTaskPool(vector<Task> & task_list, vector<int> & task_IDs, int island_num, int task_num, vector<int> function_ID, int problem_ID) 
{
    for(int i = 0; i < task_num; i++)
    {
        Task task;
        task.seed = problem_ID * (i + 1);
        srand(task.seed);
        int tmp_function_ID = rand() % function_ID.size();
        //tmp_function_ID = i;
        task.function_ID = function_ID[tmp_function_ID];
        task.option = 1;
        task_list.push_back(task);
    }

    for (int i = 0; i < island_num; i++)
    {
        int tmp_task_ID = i % task_num;
        task_IDs.push_back(tmp_task_ID);
    }
    return 0;
}

vector<string> &split(const string &str, char delim, vector<string> &elems, bool skip_empty = true)
{
    istringstream iss(str);
    for (string item; getline(iss, item, delim);)
        if (skip_empty && item.empty()) continue;
        else elems.push_back(item);
        return elems;
}

int SetParameters(vector<int> &total_runs, IslandInfo &island_info, ProblemInfo &problem_info, int argc, char** argv)
{
    //---------------------------problem info----------------------------------------------------//
    problem_info.max_base_FEs = 100;
    problem_info.seed = 1;
    problem_info.max_bound = 100;
    problem_info.min_bound = -100;
    problem_info.dim = 10;
    problem_info.computing_time = 60;
    for (int i = 1; i <= 15; i++)
      total_runs.push_back(i);
    if (getParam("-dim", argv, argc))
      problem_info.dim = atoi(getParam("-dim", argv, argc));
    if (getParam("-task_num", argv, argc))
      problem_info.dim = atoi(getParam("-dim", argv, argc));
    if (getParam("-task_num", argv, argc))
      problem_info.task_num = atoi(getParam("-task_num", argv, argc));
     if (getParam("-problem_ID", argv, argc))
      problem_info.problem_ID = atoi(getParam("-problem_ID", argv, argc));
    if (getParam("-total_runs", argv, argc))
    {
      total_runs.clear();
      string str = getParam("-total_runs", argv, argc);
      vector<string> runs;
      split(str, '-', runs);
      const char *tmp_run1 = runs[0].c_str();
      const char *tmp_run2 = runs[1].c_str();

      for (int i = atoi(tmp_run1); i <= atoi(tmp_run2); i++)
          total_runs.push_back(i);

    }
    if (getParam("-computing_time", argv, argc))
      problem_info.computing_time = atoi(getParam("-computing_time", argv, argc));
    if (getParam("-max_base_FEs", argv, argc))
      problem_info.max_base_FEs = atoi(getParam("-max_base_FEs", argv, argc));
//---------------------------node info----------------------------------------------------//

//---------------------------island info----------------------------------------------------//
    island_info.interval = 100;
    island_info.island_num = 64;
    island_info.island_size = 64;
    island_info.migration_size = 2;
    island_info.buffer_capacity = 8;
    island_info.migration_rate = 0.5;
    island_info.migration_topology.assign("dynamicConnect");
    island_info.buffer_manage.assign("DP");
    island_info.configure_EA.assign("constant");
    island_info.regroup_option.assign("null");
    island_info.replace_policy.assign("worst");

    island_info.subisland_num = 1;//island_info.island_size / subisland_size;

    if (getParam("-pop_size", argv, argc))
        island_info.island_size = atoi(getParam("-pop_size", argv, argc)) / island_info.island_num;
    if (getParam("-island_size", argv, argc))
        island_info.island_size = atoi(getParam("-island_size", argv, argc));
    if (getParam("-island_num", argv, argc))
        island_info.island_num = atoi(getParam("-island_num", argv, argc));
    if (getParam("-interval", argv, argc))
        island_info.interval = atoi(getParam("-interval", argv, argc));
    if (getParam("-migration_size", argv, argc))
        island_info.migration_size = atoi(getParam("-migration_size", argv, argc));
    if (getParam("-migration_topology", argv, argc))
        island_info.migration_topology = getParam("-migration_topology", argv, argc);
    if (getParam("-buffer_manage", argv, argc))
        island_info.buffer_manage = getParam("-buffer_manage", argv, argc);
    if (getParam("-replace_policy", argv, argc))
        island_info.replace_policy = getParam("-replace_policy", argv, argc);
    if (getParam("-configure_EA", argv, argc))
        island_info.configure_EA = getParam("-configure_EA", argv, argc);
    if (getParam("-buffer_capacity", argv, argc))
        island_info.buffer_capacity = atoi(getParam("-buffer_capacity", argv, argc));
    if (getParam("-migration_rate", argv, argc))
        island_info.migration_rate = atof(getParam("-migration_rate", argv, argc));
    
    if (getParam("-task_num", argv, argc))
    {
        vector<int> function_ID;
        function_ID.push_back(1);
        function_ID.push_back(2);
        function_ID.push_back(3);
        function_ID.push_back(4);
        if(island_info.island_num < problem_info.task_num)
            printf("error: too few island number\n");
        else
            generateTaskPool(problem_info.task_list, problem_info.task_IDs, island_info.island_num, problem_info.task_num, function_ID, problem_info.problem_ID);
    }

    return 0;

}

int ConstructAndExecuteIslandModule(vector<int> &total_runs, IslandInfo island_info, ProblemInfo problem_info)
{
    IslandEA island_EA;
    for (int i = 0; i < total_runs.size(); i++)
    {
        problem_info.run_ID = total_runs[i];
        problem_info.seed = problem_info.run_ID * problem_info.problem_ID;
        island_EA.Initilize(island_info, problem_info);
        island_EA.Execute();
        island_EA.Unitilize();
    }
    return 0;
}

int main(int argc, char* argv[])
{

    double start_time, finish_time;

    MPI_Init(&argc, &argv);

    IslandInfo island_info;
    ProblemInfo problem_info;

    start_time = MPI_Wtime();
    vector<int> total_runs;

    SetParameters(total_runs, island_info, problem_info, argc, argv);

    ConstructAndExecuteIslandModule(total_runs, island_info, problem_info);

    finish_time = MPI_Wtime();
    MPI_Finalize();

    printf("Total Elapsed time: %f seconds\n", finish_time - start_time);
    return 0;
}
