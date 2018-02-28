// Author: pmgardias
#include <stdio.h>
#include <stdlib.h>
#include "bathroom.h"

// add occupant to the bathroom instance
void addOcc(bathroom_t* br) {
	br->use_ct++;
	br->curr_occ++;
	br->tot_occ += br->curr_occ;
}

// removes occupant from the bathroom instance
void removeOcc(bathroom_t* br) {
	br->curr_occ--;
}

// changes the current state of the bathroom instance
void changeState(bathroom_t* br, int state) {
	br->state = state;
}

// calculate elapsed time in ms given timeval structs for start and end time
long elapsedTimeCalc(struct timeval* start, struct timeval* end) {
	long long elapsed_time = (end->tv_sec - start->tv_sec) * 1000;
	if (start->tv_usec > end->tv_usec) {
		elapsed_time += (start->tv_usec - end->tv_usec) / 1000;
	} else {
		elapsed_time += (end->tv_usec - start->tv_usec) / 1000;
	}
	return elapsed_time;
}

_Bool Enter(bathroom_t* br, gender g) {
	_Bool wait_flag = 0;
	pthread_mutex_lock(&(br->mutex)); // thread waits here if lock is held
	if (br->state == VACANT) {
		/* Bathroom is not occupied */
		gettimeofday(&(br->unocc_final), NULL);
		// add this previous unoccupied time to the total unoccupied time
		br->unocc_time += elapsedTimeCalc(&(br->unocc_start), &(br->unocc_final));
		changeState(br, g);
		addOcc(br);
	} else if (br->state == g) {
		/* Bathroom is occupied by the same gender */
		addOcc(br);
	} else {
		/* Bathroom is occupied by the other gender */
		wait_flag = 1;
		br->curr_queue++;
		br->tot_queue += br->curr_queue;
		br->queue_use_ct++;
		pthread_cond_wait(&(br->cond), &(br->mutex)); // thread waits
		
		changeState(br, g);
		addOcc(br);
	}
	pthread_mutex_unlock(&(br->mutex));
	return wait_flag;
}

void Leave(bathroom_t* br) {
	pthread_mutex_lock(&(br->mutex));
	removeOcc(br);
	if (br->curr_occ <= 0) {
		changeState(br, VACANT);
		gettimeofday(&(br->unocc_start), NULL);
		pthread_cond_broadcast(&(br->cond));
		br->curr_queue = 0;
	}
	pthread_mutex_unlock(&(br->mutex));
}

void Initialize(bathroom_t* br) {
	pthread_mutex_init(&(br->mutex), NULL);
	pthread_cond_init(&(br->cond), NULL);
	br->use_ct = 0;
	br->curr_occ = 0;
	br->unocc_time = 0;
	br->occ_time = 0;
	br->avg_queue_len = 0;
	br->avg_user_occ = 0;
	br->state = VACANT;
	gettimeofday(&(br->unocc_start), NULL);
	gettimeofday(&(br->start_time), NULL);
}

void Finalize(bathroom_t* br) {
	gettimeofday(&(br->final_time), NULL);
	br->run_time = elapsedTimeCalc(&(br->start_time), &(br->final_time));
	br->occ_time = br->run_time - br->unocc_time;
	br->avg_queue_len = br->tot_queue / br->queue_use_ct;
	br->avg_user_occ = br->tot_occ / br->use_ct;

	pthread_mutex_destroy(&(br->mutex));
	pthread_cond_destroy(&(br->cond));

	// Print statistics of bathroom
	printf("\nBathroom statistics:\n");
	printf("\tNumber of uses: %ld\n", (long) br->use_ct);
	printf("\tTotal time unoccupied: %ld ms\n", br->unocc_time);
	printf("\tTotal time occupied: %ld ms\n", br->occ_time);
	printf("\tAverage queue length: %d\n", br->avg_queue_len);
	printf("\tAverage occupants: %d\n", br->avg_user_occ);
}