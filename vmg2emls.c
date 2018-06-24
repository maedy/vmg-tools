#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

void parse_vmg (FILE *vmg_file, char *dir_name);
void create_file (FILE *vmg_file, char *dir_name, int sms_flag, int inet_flag, int filename_count);
void usage ();

const int BUF_SIZE = 128;
const char *X_IRMC_TYPE = "X-IRMC-TYPE:";
const char *SMS = "SMS\r\n";
const char *INET = "INET\r\n";
const char *BEGIN_VBODY = "BEGIN:VBODY\r\n";
const char *END_VBODY = "END:VBODY\r\n";

int main (int argc, char **argv) {

	char *vmg_file_name;
	char *output_direcotry_name;
	FILE *vmg_file;

	if (argc < 3) {
		usage ();
		exit (1);
	}

	vmg_file_name = argv[1];
	output_direcotry_name = argv[2];

	if (mkdir (output_direcotry_name, 0775) < 0) {
		if (errno == 17) {
			printf ("%s exists.\n", output_direcotry_name);
		} else {
			perror (output_direcotry_name);
		}
		exit (1);
	}

	if ((vmg_file = fopen (vmg_file_name, "r")) != NULL) {
		parse_vmg (vmg_file, output_direcotry_name);
	} else {
		perror (vmg_file_name);
		exit (1);
	}

	fclose (vmg_file);

	return 0;
}

void parse_vmg (FILE *vmg_file, char *dir_name) {

	char line[BUF_SIZE];
	char *p;
	int sms_flag = 0;
	int inet_flag = 0;
	int filename_count = 0;

	while (fgets (line, BUF_SIZE, vmg_file)) {
		if (line[strlen (line) - 1] == '\n') {
			if (strncmp (line, X_IRMC_TYPE, strlen (X_IRMC_TYPE)) == 0) {
				p = line + strlen (X_IRMC_TYPE);
				if (strcmp (p, SMS) == 0) {
					sms_flag = 1;
					continue;
				} else if (strcmp (p, INET) == 0) {
					inet_flag = 1;
					continue;
				}
			}
			if (strcmp (line, BEGIN_VBODY) == 0) {
				if (sms_flag || inet_flag) {
					filename_count++;
					create_file (vmg_file, dir_name, sms_flag, inet_flag, filename_count);
					sms_flag = 0;
					inet_flag = 0;
				}
			}
		}
	}
}

void create_file (FILE *vmg_file, char *dir_name, int sms_flag, int inet_flag, int filename_count) {

	char filename[32];
	char line[BUF_SIZE];
	FILE *output_file;

	if (sms_flag) {
		sprintf (filename, "%s/%d.txt", dir_name, filename_count);
	} else if (inet_flag) {
		sprintf (filename, "%s/%d.eml", dir_name, filename_count);
	} else {
		exit (1);
	}

	if ((output_file = fopen (filename, "w+")) == NULL) {
		perror (filename);
		exit (1);
	}

	while (fgets (line, BUF_SIZE, vmg_file)) {
		if (line[strlen (line) - 1] == '\n') {
			if (strcmp (line, END_VBODY) == 0) {
				break;
			}
		}
		fprintf (output_file, "%s", line);
	}

	fclose (output_file);
}


void usage () {
	printf ("You need to specify both the VMG file and its output directory.\n");
}

