#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAXLINE 512


#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib") // Winsock Library

FILE* server_log;
HANDLE mutex;
SOCKET connect_socket;
struct queue* q;
float mu;
int seed, run_id, QSize;
short port;
bool stop_connection = FALSE;

static int	read_cnt;
static char* read_ptr;
static char	read_buf[MAXLINE];

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

void update_log() {
	clock_t time = clock();
	double total_t = (double)(time) / CLOCKS_PER_SEC;
	fprintf(server_log, "%f %d\n", total_t, q->itemCount);
}


DWORD WINAPI job_handler(void* param) {
	srand(seed);

	while (!stop_connection) {
		//while (!isEmpty(q) || !stop_connection) {
		while (!isEmpty(q)) {
			// create random time job_time according to an exp~(mu) distribution
			float rend = (float)rand() / (float)(RAND_MAX);
			float job_time = (-1 * log(rend) / mu) * 1000;   // get the job time in milliseconds
			Sleep(job_time);

			WaitForSingleObject(mutex, INFINITE);            // locking
			int job_id = removeData(q);                      // remove job from queue and store it in job_id 
			update_log();
			char* string_job_id[MAXLINE];
			_itoa(job_id, string_job_id, 10);                // convert job_id to a string (string_job_id)  
			writen(connect_socket, "1", 1);                  // write finish job message to the socket
			writen(connect_socket, string_job_id, strlen(string_job_id)); // write the job_id to the socket
			writen(connect_socket, "\n", 1);
			ReleaseMutex(mutex);                             // unlocking
			printf(" job_id: %d - job succses\n",job_id);  // for debug

		}
	}
}

void open_file_check(char* filename, FILE* file) {
	if (file == NULL) {
		printf("An Error Has Occurred While opening the File: %s\n", filename);
		exit(1); /* terminates the program */
	}
}




int main(int argc, char* argv[])
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("Error at WSAStartup()\n");
	}

	// openning the log

	char filename[100];
	sprintf(filename, "server_%d.log", atoi(argv[3]));
	server_log = fopen(filename, "w");
	open_file_check(filename, server_log);

	struct sockaddr_in server_addr;
	char buffer[1024];


	if (argc != 6) {
		printf("ERROR, invalid number of arguments\n");
		exit(1);
	}

	port = atoi(argv[1]);
	seed = atoi(argv[2]);
	run_id = atoi(argv[3]);
	mu = atof(argv[4]);
	QSize = atoi(argv[5]);
	if (port < 0 || port > 65535 || seed < 0 || seed > 32767 || run_id < 0 || mu < 0 || QSize < 0) {
		printf("ERROR, One of the arguments is invalid\n");
		exit(1);
	}

	// create log headline
	fprintf(server_log, "server_%d.log: seed=%d, mu=%f, QSize=%d\n", run_id, seed, mu, QSize);

	// create socket
	int server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0) {
		printf("Socket ERROR\n");
		exit(1);
	}
	printf("TCP server socket created\n"); // for debug

	// Initialize the server_addr structure fileds
	memset(&server_addr, '\0', sizeof(server_addr));  // set addr as NULL
	server_addr.sin_family = AF_INET;                 // set addresses as IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY;         // set IP address, inet_addr gets string IP and convert it to binary
	server_addr.sin_port = htons(port);               // set port number, htons gets string port number and convert it to binary

	// try to bind socket to port
	int status = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (status < 0) {
		printf("Bind ERROR\n");
		exit(1);
	}
	printf("Bind to the port number: %d\n", port); // for debug
	// start Listening
	status = listen(server_sock, QSize);
	printf("Listening...\n");   // for debug
	// try to connect
	connect_socket = accept(server_sock, NULL, NULL);
	printf("Client connected\n");  // for debug
	
	int nodelay_value = 1;
	setsockopt(connect_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay_value, sizeof(nodelay_value));

	mutex = CreateMutex(NULL, FALSE, NULL); // initialize mutex lock
	q = createQueue(QSize);
	char string_job_id[MAXLINE];

	HANDLE job_thread = CreateThread(NULL, 0, job_handler, NULL, 0, NULL);
	if (job_handler == NULL) {
		printf("Thread not created\n");
	}
	
	// main loop
	while (readline(connect_socket, string_job_id, MAXLINE) != 0) {
		WaitForSingleObject(mutex, INFINITE);                     // locking
		if (!isFull(q)) {
			insert(q, atoi(string_job_id));
			update_log();                                // write the job data to the log
		}
		// send full queue message
		else {
			writen(connect_socket, "0", 1);
			writen(connect_socket, string_job_id, strlen(string_job_id));
			printf(" job_id: %d - job drop\n", atoi(string_job_id));      // for debug
		}
		ReleaseMutex(mutex);                                      // unlocking
	}
	stop_connection = TRUE;

	WaitForSingleObject(job_thread, INFINITE);
	shutdown(connect_socket, SD_BOTH);

	fclose(server_log);
	// print results to stderr
	fprintf(stderr, "server_%d.log: seed=%d, mu=%f, QSize=%d\n", run_id, seed, mu, QSize);
	int st = closesocket(server_sock);
	if (st != 0) {
		printf("ERROR, faild close the socket.\n");
		exit(1);
	}

	WSACleanup();
	return 0;
}