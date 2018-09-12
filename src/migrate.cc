#include "migrate.h"

Migrate::Migrate()
{
  
}

Migrate::~Migrate()
{

}

int Migrate::Initilize(IslandInfo island_info, ProblemInfo problem_info)
{
    island_info_ = island_info;
    problem_info_ = problem_info;
    DiversityPreserving tmp_buffer_manage(island_info.buffer_capacity);

    for(int i = 0; i < problem_info_.task_num; i++)
        buffer_manage_.push_back(tmp_buffer_manage);
    

    return 0;
}

int Migrate::Unitilize()
{
     buffer_manage_.clear();
    
    return 0;
}

int Migrate::UpdatePopulation(Population &population, int current_task_ID)
{
	int count = 0;

    Population emigration_import;
    buffer_manage_[current_task_ID].SelectFromBuffer(emigration_import, island_info_.migration_size);

    int worst_fitness_value_ID = 0;
    
    if (island_info_.replace_policy == "worst")
    {
        for (int i = 0; i < emigration_import.size(); i++)
        {
            int worst_ID = 0;
            real worst_value = 0;
            
            for (int j = 0; j < population.size(); j++)
            {
                if(worst_value < population[j].fitness_value)
                {
                    worst_value = population[j].fitness_value;
                    worst_ID = j;
                }
            }
            if(worst_value > emigration_import[i].fitness_value)
            {
                population[worst_ID] = emigration_import[i]; 
                count++;
            }
        }
    }
    else if (island_info_.replace_policy == "tournament")
    {
        vector<int> random_IDs = random_.Permutate(population.size(), emigration_import.size());
        for (int i = 0; i < emigration_import.size(); i++)
        {
            if(population[random_IDs[i]].fitness_value > emigration_import[i].fitness_value)
            {
                population[random_IDs[i]] = emigration_import[i]; 
                count++;
            }
        }  
    }
    return count;
}
int Migrate::EvaluateFitnessForOtherTask(vector< vector<Population> > &emigrations_pool, vector<Population> &population, EA_CPU *EA_CPU)
{
    for(int i = 0; i < island_info_.island_num; i++)
    {
        vector<Population> tmp_emigrations_pool;
        for(int j = 0; j < problem_info_.task_num; j++)
        {
            Population tmp_population;
            Population tmp_tmp_population;
            tmp_tmp_population = population[i];
            for (int k = 0; k < population[i].size(); k++)
            	tmp_tmp_population[k].fitness_value = EA_CPU->EvaluateFitness(tmp_tmp_population[k], problem_info_.task_IDs[j]);
            

            for(int k = 0; k < island_info_.migration_size; k++)
            {
                int tmp_best_index = 0;
                real tmp_best_fitness_value = tmp_tmp_population[0].fitness_value;
                for(k = 1; k < tmp_tmp_population.size(); k++)
                {
                    if(tmp_best_fitness_value > tmp_tmp_population[k].fitness_value)
                    {
                        tmp_best_index = k;
                        tmp_best_fitness_value = tmp_tmp_population[k].fitness_value;
                    }
                }
                tmp_population.push_back(tmp_tmp_population[tmp_best_index]);
                tmp_tmp_population[tmp_best_index].fitness_value = 1e20;
            }
            tmp_emigrations_pool.push_back(tmp_population);
        }
        emigrations_pool.push_back(tmp_emigrations_pool);
    }

    return 0;
}

int Migrate::EvaluateFitnessForOtherTask(Population &emigrations_pool, Population &population, EA_CPU *EA_CPU, int target_island_ID)
{
    Population tmp_population = population;
    for (int i = 0; i < population.size(); i++)
       tmp_population[i].fitness_value = EA_CPU->EvaluateFitness(tmp_population[i], problem_info_.task_IDs[target_island_ID]);
            
    for(int i = 0; i < island_info_.migration_size; i++)
    {
        int tmp_best_index = 0;
        real tmp_best_fitness_value = tmp_population[0].fitness_value;
        for(int j = 1; j < tmp_population.size(); j++)
        {
            if(tmp_best_fitness_value > tmp_population[j].fitness_value)
            {
                tmp_best_index = j;
                tmp_best_fitness_value = tmp_population[j].fitness_value;
            }
        }
        emigrations_pool.push_back(tmp_population[tmp_best_index]);
        tmp_population[tmp_best_index].fitness_value = 1e20;
    }
    return 0;
}

int Migrate::MigrateBetweenTasks(vector<int> &success_count, vector<Population> &population, EA_CPU *EA_CPU)
{
    if (island_info_.migration_rate * island_info_.island_num > problem_info_.task_num)
    {
        vector< vector<Population> > emigrations_pool;
        EvaluateFitnessForOtherTask(emigrations_pool, population, EA_CPU);
        for(int i = 0; i < island_info_.island_num; i++)
        {
            for(int j = 0; j < island_info_.island_num; j++)
                if(random_.RandRealUnif(0,1) < island_info_.migration_rate && i != j)
                    buffer_manage_[j].UpdateBuffer(emigrations_pool[i][j]);
            success_count[i] += UpdatePopulation(population[i], i);
        }   
    }
    else
    {
        for(int i = 0; i < island_info_.island_num; i++)
        {
            for(int j = 0; j < island_info_.island_num; j++)
            {
                if(random_.RandRealUnif(0,1) < island_info_.migration_rate && i != j)
                {
                    Population  emigrations_pool;
                    EvaluateFitnessForOtherTask(emigrations_pool, population[i], EA_CPU, j);
                    buffer_manage_[j].UpdateBuffer(emigrations_pool);
                }
            }
            success_count[i] += UpdatePopulation(population[i], i);
        }    
    }


    return 0;
}
