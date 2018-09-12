#ifndef __ISLANDEA_H__
#define __ISLANDEA_H__
#pragma once
#include "config.h"
#include "random.h"
#include "EA_CPU.h"
#include "migrate.h"

class IslandEA
{
private:
	EA_CPU	*				EA_CPU_;
	Random                  random_;
	Migrate 				migrate_;

	ProblemInfo				problem_info_;
	IslandInfo				island_info_;

    vector<Population>		sub_population_;
    vector<real>			time_islands_;
    vector<int>				FEs_islands_;
    vector<int>				success_count_islands_;

    string 					file_name_;
    string 					debug_file_name_;
	int  					CheckAndCreatRecordFile();
	int 					PrintResult(real time, long int current_FEs);
	int 					RunEA(real time, int FEs, vector<int> success_count);
public:
							IslandEA();
							~IslandEA();
	int 					Initilize(IslandInfo island_info, ProblemInfo problem_info);
	int 					Unitilize();
	int						Execute();
};

#endif
