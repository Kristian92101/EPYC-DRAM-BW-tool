#include <math.h>
#include <vector>
#include <stdlib.h>
#include "epycBWTool.hpp"


int main(int argc, char* argv[]) {

	// Size of the testing task arrays in MB
	int mb = 16;

	if(argc > 1) mb = atoi(argv[1]);

	// std::cout << "Running A = A + B benchmark for " << mb << " MB arrays:\n";
	size_t vec_size = mb * (1024ul*1024ul) / 8ul;
	size_t clean_size = 512ul * (1024ul*1024ul) / 8ul;
	double a [vec_size];
	double b [clean_size];
	double tmp = 0;
	
	for (unsigned long i=0; i< vec_size; i++)
	{
		a[i] = 0.2f * (float) i;
	}

	for (unsigned long i=0; i< clean_size; i++)
	{
		b[i] = 0.2f * (float) i;
	}

	for (unsigned long i=0; i< clean_size; i++)
	{
		tmp += b[i];
	}


	int arr[2] = {0,0};
	select_sets(arr);
	stop_measuring();
	reset_counters();
	start_measuring();

	// Calculations
	////////////////////////////////////////////////////////////////////////////////
	for (unsigned long i=0; i< vec_size; i++)
	{
		tmp += a[i];
	}
	////////////////////////////////////////////////////////////////////////////////
	
	stop_measuring();
	print_bandwidth();
	reset_counters();


	// std::cout << a[0];
	// std::cout << tmp;

}
