// Author: pmgardias
#ifndef BATHROOM_H
#define BATHROOM_H

#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define VACANT -1 // other states are male- and female-occupied

typedef enum { male, female } gender;

typedef struct __bathroom_t {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	
	uint use_ct; // use count
	uint queue_use_ct; // use count of queue

	uint curr_occ; // current occupants
	uint curr_queue; // current length of queue

	uint tot_occ; // total occupants of the bathroom
	uint tot_queue; // total users that have been in queue

	long unocc_time; // unoccupied time
	long occ_time; // occupied time
	long run_time; // running time of bathroom

	int avg_queue_len;
	int avg_user_occ;

	struct timeval start_time;
	struct timeval final_time;
	struct timeval unocc_start;
	struct timeval unocc_final;

	int state; // current state of bathroom (gender using bathroom or VACANT)
} bathroom_t;

long elapsedTimeCalc(struct timeval* start, struct timeval* end);
_Bool Enter(bathroom_t* br, gender g);
void Leave(bathroom_t* br);
void Initialize(bathroom_t* br);
void Finalize(bathroom_t* br);

#endif