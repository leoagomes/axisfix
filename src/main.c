#include "configurator.h"
#include "fix.h"

#include <stdio.h>
#include <getopt.h>
#include <string.h>

void print_help(char*);
void print_usage(char*);

int main(int argc, char** argv) {
	if (argc < 3) {
		print_usage(argv[0]);
		return 0;
	} else if (strcmp("help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0) {
		print_help(argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "fix") == 0) {
		af_start_fix(argv[2]);
	} else if (strcmp(argv[1], "config") == 0) {
		af_start_config(argv[2]);
	} else {
		print_usage(argv[0]);
	}

	return 0;
}

void print_usage(char* argv0) {
	printf("Usage: %s <mode> <file>\n", argv0);
	printf("examples:\n%s config pads.ini\n%s fix pads.ini\n", argv0, argv0);
}

void print_help(char* argv0) {
	print_usage(argv0);
	printf("\n<mode>:\n\tconfig\t- pad configuration mode.\n");
	printf("\tfix\t- axisfix mode.\n");
	printf("<file> is expected to be the configuration file, either to save to or to read from.\n");
}