#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

void parse_vmg (FILE *vmg_file, FILE *mbox_file, char *mail_address);
void create_file (FILE *vmg_file, FILE *mbox_file, char *mail_address);
void usage ();

const int BUF_SIZE = 128;
const char *X_IRMC_TYPE = "X-IRMC-TYPE:";
const char *SMS = "SMS\r\n";
const char *BEGIN_VBODY = "BEGIN:VBODY\r\n";
const char *FROM_ = "From:";
const char *DATE_ = "Date:";
const char *END_VBODY = "END:VBODY\r\n";

int main (int argc, char **argv) {

	char *vmg_file_name;
	char *mbox_file_name;
	char *mail_address;
	FILE *vmg_file, *mbox_file;

	if (argc < 4) {
		usage ();
		exit (1);
	}

	vmg_file_name = argv[1];
	mbox_file_name = argv[2];
	mail_address = argv[3];

	if ((mbox_file = fopen (mbox_file_name, "w+")) == NULL) {
		perror (vmg_file_name);
		exit (1);
	}

	if ((vmg_file = fopen (vmg_file_name, "r")) != NULL) {
		parse_vmg (vmg_file, mbox_file, mail_address);
	} else {
		perror (vmg_file_name);
		exit (1);
	}

	fclose (vmg_file);
	fclose (mbox_file);

	return 0;
}

void parse_vmg (FILE *vmg_file, FILE *mbox_file, char *mail_address) {

	char line[BUF_SIZE];
	char *p;
	int sms_flag = 0;

	while (fgets (line, BUF_SIZE, vmg_file)) {
		if (line[strlen (line) - 1] == '\n') {
			if (strncmp (line, X_IRMC_TYPE, strlen (X_IRMC_TYPE)) == 0) {
				p = line + strlen (X_IRMC_TYPE);
				if (strcmp (p, SMS) == 0) {
					sms_flag = 1;
					continue;
				}
			}
			if (strcmp (line, BEGIN_VBODY) == 0) {
				if (sms_flag) {
					sms_flag = 0;
					continue;
				} else {
					create_file (vmg_file, mbox_file, mail_address);
				}
			}
		}
	}
}

void create_file (FILE *vmg_file, FILE *mbox_file, char *mail_address) {

	char line[BUF_SIZE];
	char headers[BUF_SIZE * 4];
	char buf[BUF_SIZE];
	char time_stamp[BUF_SIZE];
	char *p, *q;
	struct tm stm;


	while (fgets (line, BUF_SIZE, vmg_file)) {
		if (line[strlen (line) - 1] == '\n') {

			if (strncmp (line, FROM_, strlen (FROM_)) == 0) {
				if (strlen (FROM_) + 2 == strlen (line)) {
					sprintf (headers, "From: %s\r\n", mail_address);
				} else {
					strncpy (buf, line + strlen (FROM_) + 1, strlen (line + strlen (FROM_) + 1) - 2);
					mail_address = buf;
					sprintf (headers, "%s", line);
				}
				p = headers + strlen (headers);
				while (fgets (line, BUF_SIZE, vmg_file)) {
					sprintf (p, "%s", line);
					p += strlen (line);
					if (strncmp (line, DATE_, strlen (DATE_)) == 0) {
						q = line + strlen (DATE_) + 1;
						strptime (q, "%a, %d %b %Y %H:%M:%S %z%n", &stm);
						break;
					}
				}

				strftime (time_stamp, sizeof (time_stamp), "%a %b %d %H:%M:%S %Y", &stm);
				fprintf (mbox_file, "From %s %s\r\n", mail_address, time_stamp);
				fprintf (mbox_file, "Return-Path: <%s> \r\n", mail_address);
				fprintf (mbox_file, "%s", headers);
				continue;
			}

			if (strcmp (line, END_VBODY) == 0) {
				fprintf (mbox_file, "\r\n");
				break;
			}
		}
		fprintf (mbox_file, "%s", line);
	}
}


void usage () {
	printf ("You need to specify the VMG file, its output file name and your mail address.\n");
}

