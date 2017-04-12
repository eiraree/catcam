#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct thread_data {
	char	buff_IP_list [20];
	char 	nmap_command [255];
	FILE * 	fd_nmap;
};


void * thread_func (void * thread_param) {

	struct thread_data * thread_struct = (struct thread_data *) thread_param;
	snprintf (thread_struct->nmap_command, sizeof(thread_struct->nmap_command), "nmap -p %s --open -oG - %s | awk -F'/| '  '/Ports.*tcp/ { print $2 \" \" $4 }' ",  "8080,80",  thread_struct->buff_IP_list);

	thread_struct->fd_nmap = popen (thread_struct->nmap_command, "r");

	return thread_param;
}


int to_connect_ip_data (char * nmap_buff, struct sockaddr_in * to_connect) {

	char * temp_buff; 

	temp_buff = strtok(nmap_buff, " ");	
	if (!temp_buff)
		return 1;

	printf ("Address: %s \n", temp_buff);
	inet_aton(temp_buff, &(to_connect->sin_addr));

	temp_buff = strtok(NULL, " ");
	if (!temp_buff) {
		return 1;
	}

	to_connect->sin_port = htons(atoi(temp_buff));
	printf ("Port: %hu \n", to_connect->sin_port); 

  return 0;
} 


char * end_of_string_change (char * string) {
	if (string [strlen(string) - 1] == '\n')
		string [strlen(string) - 1] = '\0'; 
	return string;
}
