#include "island_EA.h"
IslandEA::IslandEA()
{
}

IslandEA::~IslandEA()
{

}

int IslandEA::CheckAndCreatRecordFile()
{
    file_name_ = "./Results/MTO_sequential.csv";
    ifstream exist_file;
    ofstream file;
    exist_file.open(file_name_.c_str());

    if(!exist_file)
    {
        file.open(file_name_.c_str());
        file<< "problem_ID,task_num,run_ID,time_period,total_FEs,island_num,pop_size_per_task";
        for(int i = 0; i < problem_info_.task_num; i++)
            file<<','<<"fitness_value_"<<i;
        for(int i = 0; i < problem_info_.task_num; i++)
            file<<','<<"time_task_"<<i;
        for(int i = 0; i < problem_info_.task_num; i++)
            file<<','<<"FEs_task_"<<i;
        for(int i = 0; i < problem_info_.task_num; i++)
            file<<','<<"success_count"<<i;
        file<<endl;
        file.close();
    }
    else
        exist_file.close();

#ifdef DEBUG
    stringstream tmp_task_num;
    string s_task_num;
    tmp_task_num << problem_info_.task_num;
    tmp_task_num >> s_task_num;
    debug_file_name_ = "./Results/Debug/problems_task_num=" + s_task_num + ".csv";
    EA_CPU_->PrintTaskDetails(debug_file_name_);
#endif
    return 0;
}

int IslandEA::Initilize(IslandInfo island_info, ProblemInfo problem_info)
{
    problem_info_ = problem_info;
    island_info_ = island_info;
    EA_CPU_ = new DE_CPU();

    EA_CPU_->Initilize(island_info_, problem_info_);
    srand(problem_info_.seed);
    CheckAndCreatRecordFile();

    migrate_.Initilize(island_info_, problem_info_);
    for(int i = 0; i < island_info_.island_num; i++)
    {
        Population tmp_population;
        EA_CPU_->InitilizePopulation(tmp_population, problem_info_.task_IDs[i]);
        sub_population_.push_back(tmp_population);
        time_islands_.push_back(0);
        FEs_islands_.push_back(0);
    }

    return 0;
}

int IslandEA::Unitilize()
{
    sub_population_.clear();
    time_islands_.clear();
    FEs_islands_.clear();
    success_count_islands_.clear();

    migrate_.Unitilize();
    EA_CPU_->Unitilize();
    delete EA_CPU_;

    return 0;
}


int IslandEA::RunEA(real time, int FEs, vector<int> success_count)
{
    for(int i = 0; i < island_info_.island_num; i++)
    {
        EA_CPU_->Run(sub_population_[i], problem_info_.task_IDs[i]);
        real fitness_values = EA_CPU_->FindBestIndividual(sub_population_[i]).fitness_value;
        if(fitness_values < 1e-8 && FEs_islands_[i] == 0 && time_islands_[i] == 0)
        {
            time_islands_[i] = time;
            FEs_islands_[i] = FEs;
            success_count_islands_[i] = success_count[i];
        }

    }


    return 0;
}

int IslandEA::Execute()
{
    int generation = 0;
    int current_FEs = island_info_.island_size;
    double start_time = MPI_Wtime();
    real communication_time = 0;
    vector<int> success_count(island_info_.island_num, 0);

    long int total_FEs = problem_info_.max_base_FEs * problem_info_.dim;
#ifndef COMPUTING_TIME
    while(current_FEs < total_FEs)
#else
    while(MPI_Wtime() - start_time < problem_info_.computing_time)
#endif   
    {
        RunEA(MPI_Wtime() - start_time, current_FEs, success_count);

		if (generation % island_info_.interval == 0)
		{
            migrate_.MigrateBetweenTasks(success_count, sub_population_, EA_CPU_);
            current_FEs += island_info_.island_size * problem_info_.task_num;
		}

        generation++;
        current_FEs += island_info_.island_size;
    }

    PrintResult(MPI_Wtime() - start_time, current_FEs);

    return 0;

}

int IslandEA::PrintResult(real time, long int current_FEs)
{
    vector<real> fitness_values(problem_info_.task_num, 1e20);
    vector<real> times_tasks(problem_info_.task_num, 1e20);
    vector<int> FEs_tasks(problem_info_.task_num, 0);
    vector<int> success_count_tasks(problem_info_.task_num, 0);

    for(int i = 0; i < island_info_.island_num; i++)
    {
        int task_ID = problem_info_.task_IDs[i];
        real tmp_fitness_values = EA_CPU_->FindBestIndividual(sub_population_[i]).fitness_value;

        if (FEs_islands_[i] == 0)
        {
            time_islands_[i] = time;
            FEs_islands_[i] = current_FEs;
        }

        if (tmp_fitness_values < fitness_values[task_ID])
        {
            fitness_values[task_ID] = tmp_fitness_values;
            times_tasks[task_ID] = time_islands_[i];
            FEs_tasks[task_ID] = FEs_islands_[i];
            success_count_tasks[task_ID] = success_count_islands_[i];
        }
    }
    
    printf("Run: %d, Time: %.3lf s \n", problem_info_.run_ID, time);

    ofstream file;
    file.open(file_name_.c_str(), ios::app);

    int run_ID = problem_info_.run_ID;
    int dim = problem_info_.dim;

    long int total_FEs = problem_info_.max_base_FEs * problem_info_.dim;
    int pop_size_per_task = island_info_.island_num * island_info_.island_size / problem_info_.task_num;

#ifdef COMPUTING_TIME
    file<<problem_info_.problem_ID<<','<<problem_info_.task_num<<','<<run_ID<<','<<problem_info_.computing_time <<','<<total_FEs<<','<<island_info_.island_num<<','<<pop_size_per_task; 
#else
    file<<problem_info_.problem_ID<<','<<problem_info_.task_num<<','<<run_ID<<','<<time<<','<<total_FEs<<','<<island_info_.island_num<<','<<pop_size_per_task; 
#endif
    for (int i = 0; i < problem_info_.task_num; i++)
        file<<','<<fitness_values[i];
    
    for (int i = 0; i < problem_info_.task_num; i++)
    {
        if (times_tasks[i] >= 1e10)
            times_tasks[i] = 0;
        file<<','<<times_tasks[i];
    }

    for (int i = 0; i < problem_info_.task_num; i++)
        file<<','<<FEs_tasks[i];
    for (int i = 0; i < problem_info_.task_num; i++)
        file<<','<<success_count_tasks[i];
    file<<endl;
    file.close();
    return 0;
}

    


