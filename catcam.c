#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "functions.h"


int main (int argc, char **argv) {

	FILE	*fd_IP_list;
	FILE	*fd_fail;
	char	nmap_buff [255];
	char	fail_buff [124];
	int		sock = -1;
	char	content_buff [2048];
	char *	get_http = "GET / HTTP/1.0\r\n\r\n";
	int		sel_val = 0;
	struct 	sockaddr_in to_connect;
	struct 	timeval timeout;

	struct thread_data thread_param;

	pthread_mutex_t mutex;
	pthread_t tid = 0;
	fd_set rdfs;

	if (argv[2] == NULL) {
		printf ("ERROR: UNKNOWN PARAMETER! \n");
		return 1;
	}

	memset (&content_buff, 0, sizeof (content_buff));
	memset (&to_connect, 0, sizeof(to_connect));
	memset (&timeout, 0, sizeof(timeout));


	fd_IP_list = fopen (argv[1], "r");
	if (fd_IP_list == NULL) {
		printf ("ERROR: %s (FILE NOT FOUND)! \n", strerror(errno));
		return 1;
	}

	while(fgets (thread_param.buff_IP_list, sizeof(thread_param.buff_IP_list), fd_IP_list) != NULL) {

		end_of_string_change (thread_param.buff_IP_list);

		printf ("%s \n", thread_param.buff_IP_list);


		pthread_mutex_init (&mutex, NULL);

		pthread_create (&tid, NULL, &thread_func, &thread_param);

		sleep (5); 

		pthread_mutex_destroy (&mutex);

		while (fgets (nmap_buff, sizeof(nmap_buff), thread_param.fd_nmap) != NULL) {

			printf ("%s", nmap_buff);

			to_connect.sin_family = AF_INET;

			if (to_connect_ip_data (nmap_buff, &to_connect) == 1)
				goto error;

			if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
				printf ("ERROR: %s! \n", strerror(errno));
				goto error;
			}

			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

			FD_ZERO (&rdfs); 
			FD_SET (sock, &rdfs); 

			if (connect (sock, (struct sockaddr *) &to_connect, sizeof(to_connect)) == -1) {
				printf ("ERROR: %d (%s)! \n", errno, strerror(errno));
			} else {
				printf("Connected!\n");
			}

			if (send (sock, get_http, strlen(get_http), MSG_NOSIGNAL) == -1)
				printf ("ERROR: %d (%s)! \n", errno, strerror(errno));

			if ((sel_val = select (sock + 1, &rdfs, NULL, NULL, &timeout)) == -1) {
				printf ("ERROR: %s! \n", strerror(errno));
			} else if (sel_val) {
				printf ("Data is available now \n");
				if (recv(sock, content_buff, sizeof(content_buff), 0) == -1) 
					printf("ERROR: %s! \n", strerror (errno));
			} else {
				printf ("Timeout \n");
			}

			printf ("\n"); 

			fd_fail = fopen (argv[2], "r");


			while (fgets(fail_buff, sizeof(fail_buff), fd_fail) != NULL) {

				end_of_string_change (fail_buff);

				if (strstr(content_buff, fail_buff) != NULL) {
					break;
				}

			close(sock);
			}
		}

		pclose (thread_param.fd_nmap); 
		fclose (fd_IP_list);
		fclose (fd_fail);

	}

	return 0;

error:
	pclose(thread_param.fd_nmap);
	fclose(fd_IP_list);
	fclose(fd_fail);
	return 1;
}
