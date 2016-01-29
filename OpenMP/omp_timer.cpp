#include <omp.h>


double start_so, stop_so, start_po, stop_po;

start_so=omp_get_wtime();
	//do some work
stop_so=omp_get_wtime();

cout << "time seq: " << (stop_so-start_so) << "\n";
cout << "time par: " << (stop_po-start_po) << "\n";


