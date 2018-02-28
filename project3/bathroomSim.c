// Author: pmgardias
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "bathroom.h"

#define MICROSEC 1e5
#define MIN_RAND 1
#define MAX_RAND 100

bathroom_t br; // global bathroom instance

typedef struct __thread_data_t {
	int tnum;
	gender tgender;
} thread_data_t;

pthread_mutex_t print; // one thread prints at a time

/* the following values are used to base the randomized 
 * components of each thread on
 */
double avg_arr_time;
double avg_stay_time;
double avg_loop_ct;

double distributedRand(double mean) {
	double a = drand48(), b = drand48();
	double z = sqrt(-2 * log(a)) * cos(2 * M_PI * b); // Box-Muller transform
	double result = (mean / 2 * z) + mean;
	if (result < MIN_RAND) {
		return 1;
	} else if (result > MAX_RAND) {
		return (MAX_RAND - 1);
	} else {
		return result;
	}
}

void* Individual(void* data) {
	long avg_queue_time = 0;
	long max_queue_time = 0;
	long min_queue_time = 1e9; // has to start as large num
	long tot_queue_time = 0;
	long tot_wait_ct = 0;

	struct timeval start, end;
	thread_data_t* user = (thread_data_t *) data;

	int rand_loop_ct = (int) floor(distributedRand(avg_loop_ct));
	printf("Thread number %d looping %d times\n", user->tnum, rand_loop_ct);
	for (int i = 0; i < rand_loop_ct; i++) {
		int rand_arr_time = (int) floor(distributedRand(avg_arr_time));
		int rand_stay_time = (int) floor(distributedRand(avg_stay_time));
		
		// wait until arrival time
		usleep(rand_arr_time * MICROSEC);
		gettimeofday(&start, NULL);
		_Bool waited = Enter(&br, user->tgender);
		gettimeofday(&end, NULL);

		// wait until stay time ends
		usleep(rand_stay_time * MICROSEC);
		Leave(&br); // Leave bathroom
		if (waited) {
			// replace min or max queue time if necessary
			tot_wait_ct++;
			long elapsed_time = elapsedTimeCalc(&start, &end);
			tot_queue_time += elapsed_time;
			if (elapsed_time > max_queue_time) {
				max_queue_time = elapsed_time;
			}
			if (elapsed_time < min_queue_time) {
				min_queue_time = elapsed_time;
			}
		}
	}

	if (tot_wait_ct != 0) {
		avg_queue_time = tot_queue_time / tot_wait_ct;
	}
	// checks if min_queue_time has not changed
	if (min_queue_time == 1e9) {
		min_queue_time = 0;
	}
	pthread_mutex_lock(&print);
	// Print thread statistics, such as tnum, tgender, loop ct, etc.
	if (user->tgender == male) {
		printf("\nThread number %d (male) has finished running\n", user->tnum);
	} else if (user->tgender == female) {
		printf("\nThread number %d (female) has finished running\n", user->tnum);
	}
	printf("Loop count: %d\n", rand_loop_ct);
	printf("Number of waits in queue: %ld\n", tot_wait_ct);
	printf("Total queue time: %ld ms\n", tot_queue_time);
	printf("Minimum queue time: %ld ms\n", min_queue_time);
	printf("Maximum queue time: %ld ms\n", max_queue_time);
	printf("Average queue time: %ld ms\n", avg_queue_time);
	pthread_mutex_unlock(&print);
	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	int users;
	Initialize(&br);
	srand48(time(NULL));
	pthread_mutex_init(&print, NULL);
	if (argc == 5) {
		users = atoi(argv[1]);
		avg_arr_time = strtof(argv[2], (char **) NULL);
		avg_stay_time = strtof(argv[3], (char **) NULL);
		avg_loop_ct = strtof(argv[4], (char **) NULL);
	} else {
		printf("ERROR: Incorrect number of arguments passed\n");
		exit(-1);
	}

	printf("-- Simulation starting with %d users --\n", users);
	pthread_t thread[users];
	for (int i = 0; i < users; i++) {
		thread_data_t* data = (thread_data_t *) malloc(sizeof(thread_data_t *)); // current thread instance
		data->tnum = i;
		double rand_gen = drand48();
		if (rand_gen < 0.5) {
			data->tgender = male; // enum 0
		} else {
			data->tgender = female; // enum 1
		}
		int ptcerr = pthread_create(&thread[i], NULL, Individual, (void *) data);
		if (ptcerr != 0) {
			printf("ERROR: Creating thread failed with code %d\n", ptcerr);
			exit(-2);
		}
	}

	for (int i = 0; i < users; ++i) {
		pthread_join(thread[i], NULL);
	}
	
	Finalize(&br);
	return EXIT_SUCCESS;
}