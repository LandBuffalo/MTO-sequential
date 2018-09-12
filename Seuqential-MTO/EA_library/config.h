#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <cmath>
#include <time.h>
#include <numeric>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
using namespace std;

#define TOTAL_RECORD_NUM		20
//#define COMPUTING_TIME
#define DEBUG

//#define DISPLAY
//#define DIVERSITY
typedef double real;
struct Individual
{
	vector<double> elements;
	double fitness_value;
};

struct Task
{
	int function_ID;
	int option;
	int seed;
};

typedef vector<Individual> Population;

struct ProblemInfo
{
	int dim;
	int run_ID;
	int problem_ID;
	int max_base_FEs;
	int seed;
	int computing_time;
	real max_bound;
	real min_bound;
	int task_num;
	vector<Task> task_list;
	vector<int> task_IDs;
};


struct IslandInfo
{
	int island_size;
	int island_num;
	int interval;
	int migration_size;
    int buffer_capacity;
	real migration_rate;
  	int subisland_num;
    string configure_EA;
    string regroup_option;
    string migration_topology;
	string buffer_manage;
	string replace_policy;
};

struct DisplayUnit
{
	int FEs;
	double time;
	real fitness_value;
};



#endif
