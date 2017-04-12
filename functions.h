struct thread_data {
	char    buff_IP_list [20];
	char    nmap_command [255];
	FILE *  fd_nmap;
}; 


void * thread_func (void * thread_param);
char * end_of_string_change (char * string);
int to_connect_ip_data (char * nmap_buff, struct sockaddr_in * to_connect);

