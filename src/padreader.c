#include "padreader.h"
#include "debug.h"
#include "pad.h"

#include "ini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ini_read_padc(void* padc, const char* section, const char* name, const char* value) {
	if (strcmp(section, "general") == 0 && strcmp(name, "pads") == 0)
		*((int*)padc) = atoi(value);
	return 0;
}

int extract_pad_index(const char* padstr) {
	char* num;

	num = strchr(padstr, ' ');
	num++;

	return atoi(num);
}

static int ini_read_pads(void* pads, const char* section, const char* name, const char* value) {
	int pad_index;
	struct ddrpad* current_pad;

	if (strncmp(section, "pad", 3) != 0)
		return 1;

	pad_index = extract_pad_index(section);
	current_pad = &(((struct ddrpad*)pads)[pad_index]);

	if (strcmp(name, "device") == 0) {
		current_pad->device = strdup(value);
	} else if (strcmp(name, "vert_axis") == 0) {
		current_pad->vert_axis = strtol(value, NULL, 0);
	} else if (strcmp(name, "vert_up") == 0) {
		current_pad->vert_up = strtol(value, NULL, 0);
	} else if (strcmp(name, "vert_down") == 0) {
		current_pad->vert_down = strtol(value, NULL, 0);
	} else if (strcmp(name, "vert_middle") == 0) {
		current_pad->vert_mid = strtol(value, NULL, 0);
	} else if (strcmp(name, "vert_idle") == 0) {
		current_pad->vert_idle = strtol(value, NULL, 0);
	} else if (strcmp(name, "hor_axis") == 0) {
		current_pad->hor_axis = strtol(value, NULL, 0);
	} else if (strcmp(name, "hor_right") == 0) {
		current_pad->hor_right = strtol(value, NULL, 0);
	} else if (strcmp(name, "hor_left") == 0) {
		current_pad->hor_left = strtol(value, NULL, 0);
	} else if (strcmp(name, "hor_middle") == 0) {
		current_pad->hor_mid = strtol(value, NULL, 0);
	} else if (strcmp(name, "hor_idle") == 0) {
		current_pad->hor_idle = strtol(value, NULL, 0);
	} else if (strcmp(name, "up_key") == 0) {
		current_pad->up_key = strtol(value, NULL, 0);
	} else if (strcmp(name, "down_key") == 0) {
		current_pad->down_key = strtol(value, NULL, 0);
	} else if (strcmp(name, "left_key") == 0) {
		current_pad->left_key = strtol(value, NULL, 0);
	} else if (strcmp(name, "right_key") == 0) {
		current_pad->right_key = strtol(value, NULL, 0);
	}
}

struct ddrpad* read_pad_data(FILE* file, int* padc) {
	struct ddrpad* pads = NULL;

	// read number of pads
	*padc = -1;
	if (ini_parse_file(file, ini_read_padc, padc) < 0) {
		err_printf("Error reading config files.\n");
		return NULL;
	}
	if (padc <= 0) {
	    err_printf("Invalid number of pads in config file.\n");
		return NULL;
	}

	// create the pads array
	pads = (struct ddrpad*)calloc(*padc, sizeof(*pads));
	if (!pads)
		return NULL;

	fseek(file, 0, SEEK_SET);

	if (ini_parse_file(file, ini_read_pads, pads)) {
		err_printf("Error reading pads.\n");
		free(pads);
		return NULL;
	}

	return pads;
}