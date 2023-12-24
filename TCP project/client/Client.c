#define _CRT_SECURE_NO_WARNINGS
#define D_CRT_NONSTDC_NO_DEPRECATE
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAXLINE 512 
#define SUCCESS 1
#define DROP 0

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <BaseTsd.h>
#include <math.h>
#include <time.h>
#include <process.h>
#include <windows.h>
#include "link_list.h"

#pragma comment(lib,"ws2_32.lib") // Winsock Library

static int	read_cnt;
static char* read_ptr;
static char	read_buf[MAXLINE];

struct list* jobs_list;
FILE* client_log;
int client_sock;
char* ip;
short port;
int seed, run_id, T, total_pkts, total_drops;
float lambda;
clock_t start_time;
clock_t current_time;
clock_t gen_time;
clock_t end_proc_time;

bool stop_connection = FALSE;




static SSIZE_T my_read(int fd, char* ptr)
{
	if (read_cnt <= 0) {
	again:
		if ((read_cnt = recv(fd, read_buf, sizeof(read_buf), 0)) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		}
		else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

SSIZE_T readline(int fd, void* vptr, size_t maxlen)
{
	SSIZE_T	n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ((rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		}
		else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		}
		else
			return(-1);		/* error, errno set by read() */
	}
	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

SSIZE_T writen(int fd, const void* vptr, size_t n)
{
	size_t		nleft;
	SSIZE_T		nwritten;
	const char* ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = send(fd, ptr, nleft, 0)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr += nwritten;
	}
	return(n);
}

DWORD WINAPI get_messages(void* param) {
	char message_buffer[MAXLINE];
	while (readline(client_sock, message_buffer, MAXLINE) != 0) {
		int succeses_or_drop = (message_buffer[0] == '1'); // succeses = 1 , drop = 0
		int job_id = atoi(message_buffer + 1);
		if (!succeses_or_drop) {
			total_drops++;
			printf("job_id: %d - job drop\n", job_id); // for debug
		}
		/*
		if (succeses_or_drop) {
			printf("job_id: %d - job success\n", job_id); // for debug
		}
		*/
		clock_t end_time = clock();
		update_end_prog_time(jobs_list, job_id, end_time, succeses_or_drop);
		total_pkts++;
	}
}

void open_file_check(char* filename, FILE* file) {
	if (file == NULL) {
		printf("An Error Has Occurred While opening the File: %s\n", filename);
		exit(1); /* terminates the program */
	}
}

void update_log() {
	
	// open file
	char filename[100];
	sprintf(filename, "client_%d.log", run_id);
	client_log = fopen(filename, "w");
	open_file_check(filename, client_log);

	// create log headline
	fprintf(client_log, "client_%d.log: seed=%d, lambda=%f, T=%d, total_pkts=%d, total_drops=%d\n",
		run_id, seed, lambda, T, total_pkts, total_drops);

	struct node* temp = jobs_list->head;
	for (int i = 0; i < jobs_list->size ; i++) {
		fprintf(client_log, "%Lf %Lf %Lf\n", (long double)temp->gen_time/ (long double)CLOCKS_PER_SEC, 
			(long double)temp->end_prog_time / (long double)CLOCKS_PER_SEC, (long double)temp->total_time / (long double)CLOCKS_PER_SEC);
		temp = temp->next; // print in seconds format
		/*
		fprintf(client_log, "%ld %ld %ld\n", temp->gen_time, temp->end_prog_time, temp->total_time); 
		temp = temp->next; // print in miliseconds format
		*/
	}
	fclose(client_log);
}



int main(int argc, char* argv[])
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("Error at WSAStartup()\n");
	}

	jobs_list = init_list();
	struct sockaddr_in addr;
	char buffer[1024];

	// openning the log
	if (argc != 7) {
		printf("ERROR, invalid number of arguments\n");
		exit(1);
	}

	ip = argv[1];
	port = atoi(argv[2]);
	seed = atoi(argv[3]);
	run_id = atoi(argv[4]);
	lambda = atof(argv[5]);
	T = atoi(argv[6]);

	if (port < 0 || port > 65535 || seed < 0 || seed > 32767 || run_id < 0 || lambda < 0) {
		printf("ERROR, One of the arguments is invalid\n");
		exit(1);
	}

	// create socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (client_sock < 0) {
		printf("Socket ERROR\n");
		exit(1);
	}
	printf("TCP server socket created.\n");  // for debug

	int nodelay_value = 1;
	setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (char *) & nodelay_value, sizeof(nodelay_value));

	// Initialize the addr structure fileds
	memset(&addr, '\0', sizeof(addr));     // set addr as NULL
	addr.sin_family = AF_INET;             // set addresses as IPv4
	addr.sin_addr.s_addr = inet_addr(ip);  // set IP address, inet_addr gets string IP and convert it to binary
	addr.sin_port = htons(port);           // set port number, htons gets string port number and convert it to binary

	// try to connect to the server
	if (connect(client_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("ERROR, faild connect to the server.\n");
		exit(1);
	}
	printf("connected to the server\n");   // for debug

	int job_id = 0;
	char job_id_as_string[MAXLINE];

	//create trhead to get handle with messages from the server
	HANDLE message_thread = CreateThread(NULL, 0, get_messages, NULL, 0, NULL);


	start_time = clock();
	current_time = clock();
	srand(seed);
	while ((current_time - start_time) < T*CLOCKS_PER_SEC) {
		// claculate time for poisson random proccess
		float rend = (float)rand() / (float)(RAND_MAX);
		float job_time = (-1 * log(rend) / lambda) * 1000; // get the job time in milliseconds
		Sleep(job_time);

		// send the job to the server
		clock_t create_job_time = clock();
		_itoa(job_id, job_id_as_string, 10);
		writen(client_sock, job_id_as_string, strlen(job_id_as_string));
		writen(client_sock, "\n", 1);
		insert(jobs_list, job_id, create_job_time);
		job_id++;
		current_time = clock();
	}

	shutdown(client_sock, SD_SEND);
	WaitForSingleObject(message_thread, INFINITE);
	update_log();
	
	// print results to stderr
	fprintf(stderr, "client_%d.log: seed=%d, lambda=%f, T=%d, total_pkts=%d, total_drops=%d\n",
		run_id, seed, lambda, T, total_pkts, total_drops);
	int status = closesocket(client_sock);
	if (status != 0) {
		printf("ERROR, faild close the socket.\n");
		exit(1);
	}
	WSACleanup();

	return 0;
}