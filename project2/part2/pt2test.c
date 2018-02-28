#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define __NR_cs3013_syscall2 334
#define MAX_CHILD 100
#define MAX_SIB 100
#define MAX_ANC 100

typedef struct ancestry_t {
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

void part2Test(unsigned short* target_pid) {
	struct ancestry_t* response = (struct ancestry_t*) malloc(sizeof(struct ancestry_t));
	printf("Testing syscall cs3013_syscall2 with pid arg %ld\n", *target_pid);
	long err = (long) syscall(__NR_cs3013_syscall2, target_pid, response);
	if (err < 0) {
		printf("System call cs3013_syscall2 returned with error value %hu\n", err);
		return;
	}
	int i;
	printf("Iterating through children of process %hu:\n", *target_pid);
	for (i = 0; i < MAX_CHILD; i++) {
		if (response->children[i] < 1) {
			if (i == 0) {
				printf("None found\n");
			}
			break;
		}
		printf("Child with pid %hu\n", response->children[i]);
	}
	printf("Iterating through siblings of process %hu:\n", *target_pid);
	for (i = 0; i < MAX_SIB; i++) {
		if (response->siblings[i] < 1) {
			if (i == 0) {
				printf("None found\n");
			}
			break;
		}
		printf("Sibling with pid %hu\n", response->siblings[i]);
	}
	printf("Iterating through ancestors of process %hu:\n", *target_pid);
	for (i = 0; i < MAX_ANC; i++) {
		if (response->ancestors[i] < 1) {
			if (i == 0) {
				printf("None found\n");
			}
			break;
		}
		printf("Ancestor with pid %hu\n", response->ancestors[i]);
	}
}

int main(int argc, char* argv[]) {
	unsigned short test_pid;
	unsigned short* test_pid_ptr = &test_pid;
	if (argc > 1) {
		test_pid = atoi(argv[1]);
	} else {
		test_pid = getpid();
	}
	pid_t pid = fork();
	if (pid == 0) {
		sleep(10);
	} else {
		part2Test(test_pid_ptr);
	}
	return 0;
}