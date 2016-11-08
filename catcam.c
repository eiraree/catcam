#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv) {

	char buff_IP_list [20]; 
	FILE *fd_IP_list;
	FILE *fd_nmap; 
	char nmap_command [255];

	if (argv[1] == NULL) {
		printf ("ERROR: UNKNOWN PARAMETER! \n");
		return 1;
	}

	fd_IP_list = fopen (argv[1], "r");

	if (fd_IP_list == NULL) {
		printf ("ERROR: FILE NOT FOUND! \n");
		return 1;
	}

		while(fgets (buff_IP_list, 20, fd_IP_list) != NULL) {
		/*	printf ("%s", buff_IP_list); */
			
			snprintf (nmap_command, 255, "nmap -p %s --open -oG - %s", "8080, 80", buff_IP_list);

			printf ("%s", nmap_command);
			fd_nmap = popen (nmap_command, "r");
		}

	pclose (fd_nmap); 
	fclose (fd_IP_list);
}
