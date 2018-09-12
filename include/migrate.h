#ifndef __MIGRATE_HH__
#define __MIGRATE_HH__
#include <mpi.h>
#include "random.h"
#include "config.h"
#include "buffer_manage.h"
#include "EA_CPU.h"
class Migrate
{
private:
    Random                  random_;
    IslandInfo              island_info_;
    ProblemInfo             problem_info_;
    
    vector<DiversityPreserving>  buffer_manage_;
    int                     UpdatePopulation(Population &population, int current_task_ID);
    int                     EvaluateFitnessForOtherTask(Population &emigrations_pool, Population &population, EA_CPU *EA_CPU, int target_island_ID);
    int                     EvaluateFitnessForOtherTask(vector< vector<Population> > &emigrations_pool, vector<Population> &population, EA_CPU *EA_CPU);

public:
                            Migrate();
                            ~Migrate();
    int                     Initilize(IslandInfo island_info, ProblemInfo problem_info);
    int                     Unitilize();
    int                     MigrateBetweenTasks(vector<int> &success_count, vector<Population> &population, EA_CPU *EA_CPU);
};
#endif
