#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "link_list.h"


#define TRUE 1
#define FALSE 0

int* array_input_priority;
int* array_output_priority;


void open_file_check(char* filename, FILE* file) {
	if (file == NULL) {
		printf("An Error Has Occurred While opening the File: %s\n", filename);
		exit(1); /* terminates the program */
	}
}
/*
void write_to_file(char* filename, FILE* file, int r) {
	if (file == NULL) {
		printf("An Error Has Occurred While opening the File: %s\n", filename);
		exit(1); 
	}
}
*/


int buffers_are_empty(struct list*** input_buffers, int N) {
	int res = TRUE;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (!is_empty(input_buffers[i][j]))
				return FALSE;
		}
	}
	return res;
}




//complete the algorithm
void islip(struct list*** input_buffers, int current_time_step , int k , int N) {
	int* array_matches = malloc(sizeof(int)*N);
	/*	array_input_priority = calloc(N, sizeof(int));
	array_outpot_priority = calloc(N, sizeof(int));*/
	int *array_grants = malloc(sizeof(int) * N);
	int* array_output_matched = calloc(N , sizeof(int));

	//init the array matches to -1
	for (int i = 0; i < N; i++)
	{
		array_matches[i] = -1;
	}

	for (int j = 0; j < k; j++)
	{
		for (int i = 0; i < N; i++)
		{
			array_grants[i] = -1;
		}
		for (int output_port = 0; output_port < N; output_port++)
		{
			if (array_output_matched[output_port])//chack if the output port is matched already.
			{
				break;
			}
			for (int input_port_relative = 0; input_port_relative < N; input_port_relative++)
			{
				int input_port = (array_output_priority[output_port] + input_port_relative) % N;
				if (!is_empty(input_buffers[input_port][output_port]) && array_matches[input_port] == -1)
				{
					array_grants[output_port] = input_port;
					break;
				}

			}
		}
		for (int input_port = 0; input_port < N; input_port++)
		{
			for (int output_port_relative = 0; output_port_relative < N; output_port_relative++)
			{
				int output_port = (array_output_priority[input_port] + output_port_relative) % N;
				if (array_grants[output_port] == input_port)
				{
					array_matches[input_port] = output_port;
					array_output_matched[output_port] = 1;
					array_input_priority[input_port] = (output_port + 1) % N;
					array_output_priority[output_port] = (input_port + 1) % N;
					break;
				}

			}
		}

	}

	for (int input_port = 0; input_port < N; input_port++)
	{
		int arrival_time, arrival_port, destination_port, depart_time;
		if (array_matches[input_port] != -1)
		{
			int output_port = array_matches[input_port];
			arrival_time = input_buffers[input_port][output_port]->head->arrival_time;
			arrival_port = input_buffers[input_port][output_port]->head->source_port;
			destination_port = input_buffers[input_port][output_port]->head->dest_port;
			printf("%d %d %d %d\n", arrival_time, arrival_port, destination_port, current_time_step);

			//delete the first nonde in the buffre in input_buffers[input_port][output_port]
			delete_first(input_buffers[input_port][output_port]);
		}

		
	}




}

void log_buffer_sizes(struct list*** input_buffers, FILE* log_file, int N , int current_time_step)
{

	for (int input_port = 0;input_port < N; input_port++)
	{
		for (int output_port =0 ; output_port < N; output_port++)
		{
			fprintf(log_file, "%d %d %d %d\n", current_time_step, input_port, output_port, input_buffers[input_port][output_port]->size);
		}
	}

}




int main(int argc, char* argv[]) {

	int N = atoi(argv[1]);
	int k = atoi(argv[2]);
	int r = atoi(argv[3]);
	FILE* log_file;
	char filename[30];
	if (argc < 4) {
		printf("Insufficient command line arguments.\n");
		exit(1);
	}


	array_input_priority = calloc(N, sizeof(int));
	array_output_priority = calloc(N, sizeof(int));
	if (array_input_priority == NULL || array_output_priority == NULL) {
		printf("Memory allocation failed.\n");
		exit(1);
	}


	sprintf(filename, "%d.log", r);
	log_file = fopen(filename, "w");
	open_file_check(filename, log_file);

	struct list*** input_buffers = calloc(N, sizeof(struct list**));
	for (int i = 0; i < N; i++) {
		input_buffers[i] = calloc(N, sizeof(struct list*));
		for (int j = 0; j < N; j++) {
			input_buffers[i][j] = init_list();
		}
	}

	int arrival_time;
	int source_port;
	int dest_port;
	int prev_arrival_time = 0;
	int times_steps_passed;
	int current_time_step = 0;
	
	int token = scanf("%d %d %d", &arrival_time, &source_port, &dest_port);
	while (token == 3) {
		times_steps_passed = arrival_time - prev_arrival_time;
		for (int i = 0; i < times_steps_passed; i++) {
			islip(input_buffers, current_time_step ,k , N); 
			log_buffer_sizes(input_buffers, log_file , N , current_time_step);
			current_time_step++;
		}
		prev_arrival_time = arrival_time;
		struct list* temp_list = input_buffers[source_port][dest_port];
		insert(temp_list, arrival_time, source_port, dest_port);
		token = scanf("%d %d %d", &arrival_time, &source_port, &dest_port);
	}
	
	while (!buffers_are_empty(input_buffers, N)) {
		islip(input_buffers, current_time_step, k, N);
		log_buffer_sizes(input_buffers, log_file, N, current_time_step);
		current_time_step++;
	}



	exit(0);
}