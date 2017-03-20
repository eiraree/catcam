#include <stdio.h>
//#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

void * thread_func (void * thread_param) {

		struct thread_data * thread_struct = (struct thread_data *) thread_param;
		snprintf (thread_struct->nmap_command, sizeof(thread_struct->nmap_command), "nmap -p %s --open -oG - %s | awk -F'/| '  '/Ports.*tcp/ { print $2 \" \" $4 }' ",  "8080,80",  thread_struct->buff_IP_list);

		thread_struct->fd_nmap = popen (thread_struct->nmap_command, "r");
}


int main (int argc, char **argv) {

	//char	buff_IP_list [20]; 
	FILE	*fd_IP_list;
	//FILE	*fd_nmap;
	FILE	*fd_fail;
	char	nmap_command [255];
	char	nmap_buff [255];
	char	fail_buff [124];
	int		sock = -1;
	char * 	temp_buff;
	char	content_buff [2048];
	char *	get_http = "GET / HTTP/1.0\r\n\r\n";
	int		sel_val = 0;
	int		mut_res = 0;
	struct 	sockaddr_in to_connect;
	struct 	timeval timeout;

	struct thread_data {
		char	buff_IP_list [20];
		char 	nmap_command [255];
		FILE * 	fd_nmap;
	} thread_param;


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

		while(fgets (buff_IP_list, sizeof(buff_IP_list), fd_IP_list) != NULL) {

			if (buff_IP_list [strlen(buff_IP_list) - 1] == '\n')
				buff_IP_list [strlen(buff_IP_list) - 1] = '\0'; 

			mut_res = pthread_mutex_init (&mutex, NULL);

			pthread_create (&tid, NULL, &thread_func, thread_struct);

			mut_res = pthread_mutex_destroy (&mutex);

			while (fgets (nmap_buff, sizeof(nmap_buff), fd_nmap) != NULL) {
				printf ("%s", nmap_buff);

				to_connect.sin_family = AF_INET;


				temp_buff = strtok(nmap_buff, " ");	
				if (!temp_buff)
					goto error;

				printf ("Address: %s \n", temp_buff);
				inet_aton(temp_buff, &to_connect.sin_addr);

				temp_buff = strtok(NULL, " ");
				if (!temp_buff) {
					goto error;
				}
				to_connect.sin_port = htons(atoi(temp_buff));
				printf ("Port: %hu \n", to_connect.sin_port);

				if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
					printf ("ERROR: %s! \n", strerror(errno));
					goto error;
				}

				timeout.tv_sec = 5;
				timeout.tv_usec = 0;

				FD_ZERO (&rdfs); 
				FD_SET (sock, &rdfs); 

				if (connect (sock, (struct sockaddr *) &to_connect, sizeof(to_connect)) == -1) {
					printf ("---------------------------------------->ERROR 1: %d (%s)! \n", errno, strerror(errno));
				} else {
					printf("Connected!\n");
				}

				if (send (sock, get_http, strlen(get_http), MSG_NOSIGNAL) == -1)
					printf ("----------------------------------------> ERROR 2: %d (%s)! \n", errno, strerror(errno));

				if ((sel_val = select (sock + 1, &rdfs, NULL, NULL, &timeout)) == -1) {
					printf ("ERROR: %s! \n", strerror(errno));
				} else if (sel_val) {
					printf ("Data is available now \n");
					if (recv(sock, content_buff, sizeof(content_buff), 0) == -1) 
						printf("ERROR: %s! \n", strerror (errno));
				} else {
					printf ("---------------------------> Timeout \n");
				}

				printf ("\n");

				printf ("\n");
				
				fd_fail = fopen (argv[2], "r");


				while (fgets(fail_buff, sizeof(fail_buff), fd_fail) != NULL) {
					if (fail_buff [strlen(fail_buff) - 1] == '\n')
						fail_buff [strlen(fail_buff) - 1] = '\0';

					if (strstr(content_buff, fail_buff) != NULL) {
						break;
						} else {
							printf ("_______________________________________________________________________ \n");
							printf (/* " %s \n", content_buff*/ "+ \n");
							printf ("_______________________________________________________________________ \n \n");
						}

				/*if (!counter) {
					printf ("_______________________________________________________________________ \n");
					printf ("%s \n", content_buff);
					printf ("_______________________________________________________________________ \n \n");
				} */
				
				close(sock);
			}
		}
	
	pclose (fd_nmap); 
	fclose (fd_IP_list);
	fclose (fd_fail);

	}

return 0;

error:
	pclose(fd_nmap);
	fclose(fd_IP_list);
	fclose(fd_fail);
	return 1;
}
