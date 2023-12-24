#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>


#define TRUE 1
#define FALSE 0



int main(int argc, char* argv[]) {

	int N = atoi(argv[1]);
	int T = atoi(argv[2]);
	int seed = atoi(argv[3]);
	float p = (float)atof(argv[4]);
	int flag = FALSE;
	if (argc == 6) { 
		flag = TRUE;
	}
	//add checks
	
	srand(seed);
	int arrive = FALSE;
	float prop;
	int dest_port = 0;
	for (int arrival_time = 0; arrival_time < T; arrival_time++) {
		for (int source_port = 0 ; source_port < N ; source_port++) {
			prop = (float)rand()/RAND_MAX;
			if (prop < p) {
				if (flag) {
					prop = (float)rand() / RAND_MAX;
					if (prop > 1 / 3) {
						dest_port = source_port;
					}
					else {
						dest_port = (source_port+1)%N;
					}
				}
				else { // no flag
					dest_port = rand() % N;
				}
				printf("%d %d %d\n", arrival_time, source_port, dest_port);
			}
		}
	}
	exit(0);
}