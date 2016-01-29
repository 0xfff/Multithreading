/*
 Pthread sort
 Albert Szmigielski
	aszmigie@sfu.ca
 to compile: g++ -g -lrt -D_REENTRANT par_pthreads_sort.cpp -o par_pthreads_sort -fopenmp -pthread
 usage: par_pthreads_sort <number of threads> <size>
 <number of threads> must be power of two
 

*/
#include <algorithm>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

using namespace std;


int Num_Threads=4; 	//default is 4 can be changed via argument
int size =1000000;	//default is 1M can be chaged via argument
float  * Array, * Array1;
int interval = floor(size/Num_Threads);

// errexit *******************************************************
void
errexit (const char *err_str)
{
    fprintf (stderr, "%s", err_str);
    exit (1);
}
// BARRIER *******************************************************
// from cmpt431
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
void
barrier (int expect)
{
    static int arrived = 0;

    pthread_mutex_lock (&mut);	//lock

    arrived++;
    if (arrived < expect)
        pthread_cond_wait (&cond, &mut);
    else {
        arrived = 0;		// reset the barrier before broadcast is important
        pthread_cond_broadcast (&cond);
    }

    pthread_mutex_unlock (&mut);	//unlock
}

// END OF BARRIER

/**************************    PAR MERGE    ******************************/
void par_merge(int thread_id, int num_lists, int mul, int interval){
		//merge until 1 list left
		//merge list 1&2, 3&4, ... (n-1 & n)

	mul = mul*2;
	int par_merge_threads = num_lists/2;
	if (num_lists > 1){
		if (thread_id < par_merge_threads)  
			{
				int start = interval * thread_id *mul;
				int end = interval * (thread_id+1) *mul ;
				int middle = floor(start+ (end - start)/2);
				if (thread_id == par_merge_threads-1) 
					{ end=size; }
				inplace_merge(&Array[start], &Array[middle], &Array[end]);			
			}
			num_lists = ceil(num_lists/2);
			barrier(Num_Threads);
			par_merge (thread_id, num_lists, mul, interval);			
	}		
}	
/**************************    PTHREAD SORT    ******************************/
void * pthreadSort(void * id){
	//cout << "in pthreadsort \n"; 
	int thread_id= *((long *) id);
	int start = interval * thread_id;
	int end = interval * (thread_id+1) ;
	//cout << "id: " <<  thread_id << " start: " << start << " end: "<< end <<"\n"; 
	if (thread_id == Num_Threads-1) 
		{end=size;}
	sort (&Array[start], &Array[end]);	
	//cout << "Thread " << thread_id << " par sort finished \n";
	// BARRIER
	barrier(Num_Threads);
	//Now merge
	par_merge(thread_id, Num_Threads, 1, interval);
}

void * pthreadSortTEST(void * id){
	int thread_id= *((long *) id);
	cout << " Hello from " << thread_id << endl;
}
/**************************    PAR SORT    ******************************/
void par_sort() {
	interval = floor(size/Num_Threads); 
	// break into num_threads chunks, sort each chunk
	pthread_attr_t attr;
    pthread_t *tid;
	int *id;
	long i; 

	// create threads
	id = (int *) malloc (sizeof (int) * Num_Threads);

	tid = (pthread_t *) malloc (sizeof (pthread_t) * Num_Threads);
	if (!id || !tid)
	    errexit ("out of shared memory");

	pthread_attr_init (&attr);
	pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
	//cout << "about to create threads \n"; 

	for (i = 1; i < Num_Threads; i++) {
	    id[i] = i;
	    pthread_create (&tid[i], &attr, pthreadSort,  &id[i]);
	}

	id[0]=0;
		pthreadSort(&id[0]);

	// wait for all threads to finish
	for (i = 1; i < Num_Threads; i++)
	    pthread_join (tid[i], NULL);

}

/****************************   MAIN   **********************************/
int main (int argc, char** argv) {
	double start_s, stop_s, start_p, stop_p;
	double time_s, time_p;
	int range = 100, error=0;
	

	if (argc>1)  Num_Threads= atoi(argv[1]);
	if (argc>2)  size = atoi(argv[2]);

	//cout << "size: " << size << endl;
	//cout << "num_threads: " << Num_Threads << endl;

	Array = new float [size];
	Array1 = new float [size];

	for (int i=0; i<size; i++){				
		Array[i]  = (rand()/(RAND_MAX +1.0));
		Array1[i]  = Array[i];
	}

	//time sequential sort
	//gettimeofday(&start_s, 0);
	start_s=omp_get_wtime();
		sort(&Array1[0], &Array1[size]);
	stop_s=omp_get_wtime();
	//gettimeofday(&end_s, 0);
	//cout << "seq. sort done \n";

	//time parallel sort
	start_p=omp_get_wtime();
		par_sort();
	stop_p=omp_get_wtime();
	//gettimeofday(&end_p, 0);

	//check correctness
	for (int i=0; i<size;i++){	
		if (Array[i] != Array1[i]){
			//cout << " Error " << "position "<< i ;
			error = 1;
		}
	}

	if (error) cout << "ERROR \n ";
	else {
		cout << "OK \n";
		cout << "Sorted " << size << " numbers with seq. sort in "<< stop_s-start_s << " seconds\n" ;
		cout << "Sorted " << size << " numbers with par. sort in "<< stop_p-start_p << " seconds\n" ;
		//int print_once=1;
		//if(print_once) cout << "size, sequential, parallel\n";
		//cout << Num_Threads << ", " << size << ", " << (stop_s-start_s) << ", " << (stop_p-start_p) <<"\n";
		}

	delete (Array);
	delete (Array1);

	return 0;
}
