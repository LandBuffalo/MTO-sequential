#include "CEC2014.h"

CEC2014::CEC2014()
{


}

CEC2014::~CEC2014()
{

}
double CEC2014::EvaluateFitnessWoShift(const vector<double> & elements)
{
	double fitness_value = 0;
	copy(elements.begin(), elements.end(), pop_original_);

	switch (func_id_)
	{
	case 1:
		ackley_func(pop_original_, &fitness_value, dim_, OShift, 0.32, 0);
		break;
	case 2:
		weierstrass_func(pop_original_, &fitness_value, dim_, OShift,  0.5 / 100.0, 0);
		break;
	case 3:
		griewank_func(pop_original_, &fitness_value, dim_, OShift, 6.0, 0);
		break;
	case 4:
		rastrigin_func(pop_original_, &fitness_value, dim_, OShift, 5.0 / 100.0, 0);
		break;
	default:
		printf("\nError: There are only 4 test functions in this test suite!\n");
		fitness_value = -1.0;
		break;
	}
	return fitness_value;
}
double CEC2014::EvaluateFitness(const vector<double> & elements)
{
	double fitness_value = 0;
	copy(elements.begin(), elements.end(), pop_original_);
	switch (func_id_)
	{
	case 1:
		ackley_func(pop_original_, &fitness_value, dim_, OShift, 0.32, 1);
		break;
	case 2:
		weierstrass_func(pop_original_, &fitness_value, dim_, OShift, 0.5 / 100.0, 1);
		break;
	case 3:
		griewank_func(pop_original_, &fitness_value, dim_, OShift, 6.0, 1);
		break;
	case 4:
		rastrigin_func(pop_original_, &fitness_value, dim_, OShift, 5.0 / 100.0, 1);
		break;
	default:
		printf("\nError: There are only 4 test functions in this test suite!\n");
		fitness_value = -1.0;
		break;
	}
	return fitness_value;
}


int	CEC2014::Unitilize()
{
	delete[]pop_original_;
	delete[]OShift;
	delete[]ze;
	return 0;
}

int CEC2014::Initilize(int func_id, int dim, int option, int seed)
{
	srand(seed);
	correlation_ = 0;
	func_id_ = func_id;
	dim_ = dim;
	ze = new double[dim_];
	pop_original_ = new double[dim_];
	OShift = new double[dim_];
	int test_num = 1;
	int sample_num = 1000;
	double low = 0.001;
	double high = 1;
	correlation_ = 0;
	double shift_rate = 0;
	while(correlation_ > 0.8 || correlation_ < 0.2)
	{
		shift_rate = random_.RandRealUnif(low, high);

		for(int i = 0; i < dim_; i++)
			OShift[i] = random_.RandRealUnif(shift_rate * (-100), shift_rate * 100);

		correlation_ = spearman(OShift, test_num, sample_num);

		if (correlation_ < 0.2)
			high = high / 1.1;
		if (correlation_ > 0.8)
			low = low * 1.1;
		if (low >= high)
		{
			printf("error: low (%lf) >= high (%lf)\n", low, high);
			break;
		}
	}
	//printf("%d\t%lf\t%lf\n", func_id, correlation_, shift_rate);

	return 0;
}
double CEC2014::Correlation()
{
	return correlation_;
}
double CEC2014::Shift(int element_ID)
{
	return OShift[element_ID];
}

double CEC2014::spearman(double * OShift, int test_num, int sample_num)
{
	double cor = 0;
	for(int i = 0; i < test_num; i++)
	{
		vector<double> y0, y1, rank0, rank1;
		vector<int> d;

		for(int j = 0; j < sample_num; j++)
		{
			vector<double> elements;
			for(int k = 0; k < dim_; k++)
				elements.push_back(random_.RandRealUnif(-100, 100));
			y0.push_back(EvaluateFitnessWoShift(elements));
			y1.push_back(EvaluateFitness(elements));
		}
		for(int j = 0; j < sample_num; j++)
		{
			rank0.push_back(0);
			rank1.push_back(0);
			for(int k = 0; k < sample_num; k++)
			{
				if (y0[j] > y0[k])
					rank0[j] = rank0[j] + 1;
				if(y0[j] == y0[k] && j!= k)
					rank0[j] = rank0[j] + 0.5;
				if (y1[j] > y1[k])
					rank1[j] = rank1[j] + 1;
				if(y1[j] == y1[k] && j!= k)
					rank1[j] = rank1[j] + 0.5;
			}
		}


   		for(size_t j = 0; j < sample_num; j++)
   		{
   			d.push_back(abs(rank0[j] - rank1[j]));
   			d[j] = d[j] * d[j];
   			//printf("%d\t%lf\t%lf\t%lf\t%lf\n", d[j], y0[j], y1[j], rank0[j], rank1[j]);
   		}

   		int sum = accumulate(d.begin(),d.end(), 0);
   		cor = 1 - 6.0 * sum / (sample_num * (sample_num * sample_num - 1.0) + 0.0);
   		if (cor < 0.2 || cor > 0.8)
   			return cor;	
	}
	return cor;
}

void CEC2014::ackley_func(double *x, double *f, int nx, double *Os, double rate, int shift_flag) /* Ackley's  */
{
	int i;
	double sum1, sum2;
	sum1 = 0.0;
	sum2 = 0.0;

	shiftfunc(x, ze, nx, Os, rate, shift_flag); /* shift and rotate */

	for (i = 0; i<nx; i++)
	{
		sum1 += ze[i] * ze[i];
		sum2 += cos(2.0*PI*ze[i]);
	}
	sum1 = -0.2*sqrt(sum1 / (double)nx);
	sum2 /= (double)nx;
	f[0] = E - 20.0*exp(sum1) + 20.0 - exp(sum2);
}


void CEC2014::weierstrass_func(double *x, double *f, int nx, double *Os, double rate, int shift_flag) /* Weierstrass's  */
{
	int i, j, k_max;
	double sum, sum2, a, b;
	a = 0.5;
	b = 3.0;
	k_max = 20;
	f[0] = 0.0;

	shiftfunc(x, ze, nx, Os, rate, shift_flag); /* shift and rotate */

	for (i = 0; i<nx; i++)
	{
		sum = 0.0;
		sum2 = 0.0;
		for (j = 0; j <= k_max; j++)
		{
			sum += pow(a, j)*cos(2.0*PI*pow(b, j)*(ze[i] + 0.5));
			sum2 += pow(a, j)*cos(2.0*PI*pow(b, j)*0.5);
		}
		f[0] += sum;
	}
	f[0] -= nx*sum2;
}


void CEC2014::griewank_func(double *x, double *f, int nx, double *Os, double rate, int shift_flag) /* Griewank's  */
{
	int i;
	double s, p;
	s = 0.0;
	p = 1.0;

	shiftfunc(x, ze, nx, Os, rate, shift_flag); /* shift and rotate */

	for (i = 0; i<nx; i++)
	{
		s += ze[i] * ze[i];
		p *= cos(ze[i] / sqrt(1.0 + i));
	}
	f[0] = 1.0 + s / 4000.0 - p;
}

void CEC2014::rastrigin_func(double *x, double *f, int nx, double *Os, double rate, int shift_flag) /* Rastrigin's  */
{
	int i;
	f[0] = 0.0;

	shiftfunc(x, ze, nx, Os, rate, shift_flag); /* shift and rotate */

	for (i = 0; i<nx; i++)
	{
		f[0] += (ze[i] * ze[i] - 10.0*cos(2.0*PI*ze[i]) + 10.0);
	}
}


void CEC2014::shiftfunc(double *x, double *xshift, int nx, double *Os, double shift_rate, int shift_flag)
{
	if(shift_flag == 1)
	{
		for (int i = 0; i<nx; i++)
			xshift[i] = (x[i] - Os[i]) * shift_rate;
	}
	else
	{
		for (int i = 0; i<nx; i++)
			xshift[i] = x[i] * shift_rate;
	}
}
